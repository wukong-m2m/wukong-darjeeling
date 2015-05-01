import sys, os, fcntl
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

import tornado.ioloop
import hashlib
import logging
logger = logging
logger.debug = logging.warning
import collections
import gevent
from gevent.event import AsyncResult
import gevent.queue
import wusignal
from configuration import *
from globals import *
import pynvc
import time

import struct
from gevent import socket
from tinyrpc.protocols.jsonrpc import JSONRPCProtocol
from tinyrpc.transports import ClientTransport
from tinyrpc import RPCClient
from transport import NetworkServerAgent
import mptnUtils as MPTN
import ipaddress
from gevent.lock import RLock
import json
import hashlib
import traceback

try:
    import fcntl
except ImportError:
    try:
        from ctypes import windll, WinError
    except ImportError:
        def prevent_socket_inheritance(sock):
            """Dummy function, since neither fcntl nor ctypes are available."""
            pass
    else:
        def prevent_socket_inheritance(sock):
            """Mark the given socket fd as non-inheritable (Windows)."""
            if not windll.kernel32.SetHandleInformation(sock.fileno(), 1, 0):
                raise WinError()
else:
    def prevent_socket_inheritance(sock):
        """Mark the given socket fd as non-inheritable (POSIX)."""
        fd = sock.fileno()
        old_flags = fcntl.fcntl(fd, fcntl.F_GETFD)
        fcntl.fcntl(fd, fcntl.F_SETFD, old_flags | fcntl.FD_CLOEXEC)

'''
DeferredQueue class: A queue with timeout
'''
class DeferredQueue:
    _defer_queue = None
    @classmethod
    def init(cls):
        if not cls._defer_queue:
            cls._defer_queue = DeferredQueue()
        return cls._defer_queue

    def __init__(self):
        self.queue = {}

    def removeTimeoutDefer(self):
        for key, defer in self.queue.items():
            if defer.timeout < int(round(time.time() * 1000)):
                #print 'remove timeouted defer', defer
                # call error cb
                defer.error_cb(None)
                del self.queue[key]

    def find_defer(self, deliver):
        #print 'finding defer for message in queue', self.queue
        for defer_id, defer in self.queue.items():
            if defer.verify(deliver, defer):
                print '[transport] found'
                return defer_id, defer
            else:
                print "[transport] Either one of %s \
                expected from defer %s does not match or \
                the sequence number got skewed: %s" % (str(defer.allowed_replies), str(defer), str(deliver))

        print '[transport] defer not found'
        return False, False

    def add_defer(self, defer):
        queue_id = str(len(self.queue)) + hashlib.md5(str(defer.message.destination) + str(defer.message.command)).hexdigest()
        print "[transport] adding to queue: queue_id ", str(queue_id)
        self.queue[queue_id] = defer
        return queue_id

    def remove_defer(self, defer_id):
        #print 'remove_defer'
        if defer_id in self.queue:
            #print 'removing defer', self.queue[defer_id]
            del self.queue[defer_id]
            return defer_id
        else:
            return False

    def get_defer(self, defer_id):
        if defer_id in self.queue:
            return self.queue[defer_id]
        else:
            return False

'''
Message and Defer Class
'''
Message = collections.namedtuple('Message', 'destination command payload')
Defer = collections.namedtuple('Defer', 'callback error_cb verify allowed_replies message timeout')

def new_defer(*args):
    return Defer(*args)

def new_message(*args):
    return Message(*args)

def new_deliver(*args):
    return Message(*args)

'''
A transport_agent_out_tasks queue and a message queue
'''
transport_agent_out_tasks = gevent.queue.Queue()
broker_messages = gevent.queue.Queue()


'''
TransportAgent abstract class
'''
class TransportAgent:
    def __init__(self):
        self._seq = 0
        gevent.spawn(self.handler)
        gevent.spawn(self.receive)

    def getNextSequenceNumberAsPrefixPayload(self):
      self._seq = (self._seq + 1) % (2**16)
      return [self._seq/256, self._seq%256]

    # to be overridden, non-blocking, send defer to greelet thread
    def deferSend(self, destination, command, payload, allowed_replies, cb):
        pass

    # to be overridden, blocking before it returns
    def send(self, destination, command, payload, allowed_replies):
        pass

    # could be overridden
    def verify(self, allowed_replies):
        return lambda deliver, defer: (deliver.command in allowed_replies) and (deliver.payload is not None and deliver.payload[0:2]==defer.message.payload[0:2])


    # to be run in a greenlet thread, context switching with handler
    def receive(self, timeout_msec):
        pass

    # to be run in a thread, and others will use ioloop to monitor pipe of this thread
    def handler(self):
        pass

'''
RPCClientAgent connects to gateway by TCPClient
'''
class RPCTCPClient(ClientTransport):
    def __init__(self, server_id, server_address):
        self.server_id = server_id
        self.server_address = server_address

    def handle_rpcrep_message(self, packet):
        dest_id, src_id, msg_type, payload = packet

        if dest_id is None:
            logger.error("RPCREP packet is shorter than required.\n%s" % str(packet))
            return None

        if msg_type != MPTN.MPTN_MSGTYPE_RPCREP:
            logger.error("RPCREP drops message since msg_type 0x%X is not RPCREP" % msg_type)
            return None

        if dest_id != MPTN.MASTER_ID:
            logger.error("RPCREP drops RPCREP message since dest_id %s 0x%X is not master" % (MPTN.ID_TO_STRING(dest_id), dest_id))
            return None

        if not getIDService().is_id_valid(src_id):
            logger.error("RPCREP drops RPCREP message since src_id %s 0x%X is not valid" % (MPTN.ID_TO_STRING(src_id), src_id))
            return None

        logger.debug("got RPCREP message from src_id %s:\n%s" % (MPTN.ID_TO_STRING(src_id), MPTN.formatted_print([payload])))

        return payload

    def send_message(self, message, expect_reply=True):
        message = MPTN.create_packet_to_str(self.server_id, MPTN.MASTER_ID, MPTN.MPTN_MSGTYPE_RPCCMD, message)
        logger.debug("RPC sending message: \n%s" % (MPTN.formatted_print(MPTN.split_packet_to_list(message))))

        packet = MPTN.socket_send(None, self.server_id, message, expect_reply=True)
        if packet is None:
            logger.error("cannot get RPCREP from gateway ID 0x%X %s addr=%s due to network problem" % (self.server_id,
                MPTN.ID_TO_STRING(self.server_id), str(self.server_address)))
            return None

        ret = self.handle_rpcrep_message(packet)

        if expect_reply:
            return ret

class RPCAgent(TransportAgent):
    _rpc_agent = None
    @classmethod
    def init(cls):
        if not cls._rpc_agent:
            cls._rpc_agent = RPCAgent()
        return cls._rpc_agent

    def __init__(self):
        self._mode = 'stop'
        self._clients = {}
        self._protocol_handlers = {}
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_IDREQ] = MPTN.ProtocolHandler(
            MPTN.ONLY_FROM_TCP_SERVER, getIDService().handle_idreq_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_GWIDREQ] = MPTN.ProtocolHandler(
            MPTN.ONLY_FROM_TCP_SERVER, getIDService().handle_gwidreq_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_RPCREP] = MPTN.ProtocolHandler(
            MPTN.ONLY_FROM_TCP_SERVER, MPTN.handle_reply_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_RTPING] = MPTN.ProtocolHandler(
            MPTN.ONLY_FROM_TCP_SERVER, getIDService().handle_rtping_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_RTREQ] = MPTN.ProtocolHandler(
            MPTN.ONLY_FROM_TCP_SERVER, getIDService().handle_rtreq_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_FWDREQ] = MPTN.ProtocolHandler(
            MPTN.ONLY_FROM_TCP_SERVER, getIDService().handle_fwdreq_message)
        MPTN.set_message_handler(self._process_message)
        MPTN.set_self_id(MPTN.MASTER_ID)
        MPTN.set_nexthop_lookup_function(getIDService()._find_nexthop_for_id)
        self._remove_timeout_greenlet = gevent.spawn(self._remove_timeout)
        TransportAgent.__init__(self)
        gevent.sleep(0)

    def _remove_timeout(self,):
        while True:
            getDeferredQueue().removeTimeoutDefer()
            gevent.sleep(2.5)

    def _get_client_rpc_stub(self, mptn_id, address):
        logger.debug("get client RPC stub for ID %s %X addr %s" % (MPTN.ID_TO_STRING(mptn_id), mptn_id, str(address)))
        stub = self._clients.get(mptn_id)
        if stub is None:
            assert isinstance(mptn_id, int), "mptn_id should be integer"
            assert isinstance(address, tuple) and len(address) == 2, "address should be a 2-tuple"
            assert isinstance(address[0], basestring), "port should be string"
            assert isinstance(address[1], int), "port should be integer"
            rpc_client = RPCClient(JSONRPCProtocol(), RPCTCPClient(mptn_id, address))
            stub = rpc_client.get_proxy()
            self._clients[mptn_id] = stub
        return stub

    # non-blocking method
    def deferSend(self, destination, command, payload, allowed_replies, cb, error_cb):

        def callback(reply):
            cb(reply)

        def error_callback(reply):
            error_cb(reply)

        defer = new_defer(
                callback,
                error_callback,
                self.verify(allowed_replies),
                allowed_replies,
                new_message(destination, command, self.getNextSequenceNumberAsPrefixPayload() + payload),
                int(round(time.time() * 1000)) + 10000
        )

        transport_agent_out_tasks.put_nowait(defer)
        return defer

    # blocking method
    def send(self, destination, command, payload, allowed_replies):
        result = AsyncResult()

        def callback(reply):
            result.set(reply)

        def error_callback(reply):
            result.set(reply)

        defer = new_defer(
                callback,
                error_callback,
                self.verify(allowed_replies),
                allowed_replies,
                new_message(destination, command, self.getNextSequenceNumberAsPrefixPayload() + payload),
                int(round(time.time() * 1000)) + 5000
        )

        transport_agent_out_tasks.put_nowait(defer)

        return result.get()

    # blocking method
    def getDeviceType(self, node):
        result = AsyncResult()

        def callback(reply):
            result.set(reply)

        defer = new_defer(
                callback,
                callback,
                None,
                None,
                new_message(node, "device_type", 0),
                0
        )

        transport_agent_out_tasks.put_nowait(defer)

        return result.get()

    # blocking method
    def routing(self):
        result = AsyncResult()

        def callback(reply):
            result.set(reply)

        defer = new_defer(
                callback,
                callback,
                None,
                None,
                new_message(None, "routing", 0),
                0
        )

        transport_agent_out_tasks.put_nowait(defer)

        return result.get()

    # blocking method
    def discovery(self):
        result = AsyncResult()

        def callback(reply):
            result.set(reply)

        defer = new_defer(
                callback,
                callback,
                None,
                None,
                new_message(None, "discovery", 0),
                0
        )

        transport_agent_out_tasks.put_nowait(defer)

        return result.get()

    def _learn(self, to_mode):
        if to_mode not in ['stop','add','delete']:
            print "[transport] RPC calls do not have a member named %s" % to_mode
            return False

        if self._mode != 'stop' and to_mode != 'stop':
            return False

        if self._mode == 'stop' and to_mode == 'stop':
            return True

        if to_mode != 'stop':
            getIDService().start_accepting()
        else:
            getIDService().stop_accepting()

        count = 0
        for gateway in getIDService().get_all_gateways():
            gateway_stub = self._get_client_rpc_stub(gateway.id, gateway.tcp_address)
            try:
                if to_mode == 'add':
                    if gateway_stub.add():
                        count += 1
                elif to_mode == 'delete':
                    if gateway_stub.delete():
                        count += 1
                else:
                    if gateway_stub.stop():
                        count += 1
            except Exception as e:
                logger.error("RPC learn failed %s with gateway ID %s 0x%X addr=%s error=%s\n%s" % (to_mode,
                    MPTN.ID_TO_STRING(gateway.id), gateway.id, str(gateway.tcp_address), str(e), traceback.format_exc()))

        if count > 0:
            self._mode = to_mode
            return True

        logger.error("RPC learn failed into %s mode with all gateways or no gateway" % to_mode)
        getIDService().stop_accepting()
        return False

    def add(self):
        return self._learn('add')

    def delete(self):
        return self._learn('delete')

    def stop(self):
        return self._learn('stop')

    def poll(self):
        ret = []
        for gateway in getIDService().get_all_gateways():
            gateway_stub = self._get_client_rpc_stub(gateway.id, gateway.tcp_address)
            try:
                ret.append(gateway_stub.poll())
            except Exception as e:
                logger.error("RPC poll fails with gateway ID=%s 0x%X addr=%s; error=%s\n%s" % (MPTN.ID_TO_STRING(gateway.id), gateway.id,
                    str(gateway.tcp_address), str(e), traceback.format_exc()))

        if len(ret):
            return "; ".join(ret)
        else:
            logger.error("RPC poll fails with all gateways or no gateway")
            return "Not available"

    def _process_message(self, context, message):
        dest_id, src_id, msg_type, payload = MPTN.extract_packet_from_str(message)

        if dest_id is None:
            log_msg = MPTN.formatted_print(MPTN.split_packet_to_list(message))
            logger.error("RPC processing message with too short header: %s" % log_msg)
            return

        protocol_handler = self._protocol_handlers.get(msg_type)
        if protocol_handler is None:
            log_msg = MPTN.formatted_print(MPTN.split_packet_to_list(message))
            print "RPC processing unknown message type 0x%X, message:\n%s" % (msg_type, log_msg)
            return

        protocol_handler.handler(context, dest_id, src_id, msg_type, payload)
        return

    def receive(self, timeout_msec=100):
        try:
            server = socket.socket()
            prevent_socket_inheritance(server)
            server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            server.bind(('', 9010))
            server.listen(500)
        except Exception as e:
            server.close()
            logger.error("TCP Server cannot start: socket error %s\n%s" % (str(e), traceback.format_exc()))
            return

        logger.info("TCP server listens on %s" % str(server.getsockname()))
        while True:
            new_sock, address = server.accept()
            prevent_socket_inheritance(new_sock)
            logger.info("TCP Server accepts from address %s" % str(address))
            gevent.spawn(MPTN.handle_socket, new_sock, address)
            gevent.sleep(0.001)

    def handler(self):
        while True:
            defer = transport_agent_out_tasks.get()
            #print 'handler: getting defer from task queue'

            if defer.message.command == "discovery":
                #print 'handler: processing discovery request'
                discovered_nodes = []
                for gateway in getIDService().get_all_gateways():
                    # logger.debug("discover get gateway ID %s %X addr=%s" % (MPTN.ID_TO_STRING(gateway.id), gateway.id, str(gateway.tcp_address)))
                    gateway_stub = self._get_client_rpc_stub(gateway.id, gateway.tcp_address)
                    try:
                        gateway_discovered_nodes = gateway_stub.discover()
                        network_size = gateway.network_size
                        prefix = gateway.prefix
                        for node_address in gateway_discovered_nodes:
                            if node_address >= network_size:
                                logger.error("RPC discover fails with gateway ID=%s addr=%s: discovered nodes %d" % (
                                    MPTN.ID_TO_STRING(gateway.id),
                                    str(gateway.tcp_address), gateway_discovered_nodes[i])
                                )
                                continue
                            discovered_nodes.append(prefix | node_address)
                    except Exception as e:
                        logger.error("RPC discover fails with gateway ID=%s addr=%s; error=%s\n%s" % (MPTN.ID_TO_STRING(gateway.id),
                            str(gateway.tcp_address), str(e), traceback.format_exc()))
                defer.callback(discovered_nodes)

            elif defer.message.command == "routing":
                #print 'handler: processing routing request'
                routing = {}
                for gateway in getIDService().get_all_gateways():
                    gateway_stub = self._get_client_rpc_stub(gateway.id, gateway.tcp_address)
                    try:
                        r = gateway.routing()
                        network_size = gateway.network_size
                        prefix = gateway.prefix
                        neighbers = []
                        for node, l in r.items():
                            if node > network_size: continue
                            node = prefix | node
                            if len(l) == 0: break
                            for i in xrange(len(l)):
                                if l[i] >= network_size:
                                    logger.error("RPC routing fails with gateway ID=%s addr=%s: discovered_nodes %d" % (
                                        gateway.id, str(gateway.tcp_address), l[i], node))
                                    continue
                                neighbers.append(prefix | l[i])
                            routing[node] = neighbers
                    except:
                        logger.error("RPC routing fails with gateway ID=%s addr=%s; error=%s\n%s" % (MPTN.ID_TO_STRING(gateway.id), str(gateway.tcp_address), str(e), traceback.format_exc()))

                defer.callback(routing)

            elif defer.message.command == "device_type":
                device_type = None
                node_id = defer.message.destination
                node_address = getIDService().get_node_addr(node_id)
                if node_address is None:
                    device_type = [1, 255, 0]
                else:
                    gateway = getIDService().get_gateway(node_id)
                    if gateway is not None:
                        gateway = self._get_client_rpc_stub(gateway.id, gateway.tcp_address)
                        try:
                            device_type = gateway.getDeviceType(node_address)
                        except:
                            logger.error("RPC getDeviceType ID=%s fails with gateway ID=%s addr=%s; error=%s\n%s" % (MPTN.ID_TO_STRING(node_id), MPTN.ID_TO_STRING(gateway.id), str(gateway.tcp_address), str(e), traceback.format_exc()))

                defer.callback(device_type)

            else: # defer.message.command is 'send'
                retries = 1
                node_id = defer.message.destination
                command = defer.message.command
                payload = defer.message.payload

                node_address = getIDService().get_node_addr(node_id)
                gateway = getIDService().get_gateway(node_id)
                if gateway is None or node_address is None:
                    retries = 0
                else:
                    gateway_stub = self._get_client_rpc_stub(gateway.id, gateway.tcp_address)

                    # prevent pyzwave send got preempted and defer is not in queue
                    if len(defer.allowed_replies) > 0:
                        logger.info("RPC send adds defer %s to agent queue" % str(defer))
                        getAgent().append(defer)

                    while retries > 0:
                        try:
                            #print "handler: sending message from defer"
                            header = MPTN.create_packet_to_str(
                                node_id,
                                MPTN.MASTER_ID,
                                MPTN.MPTN_MSGTYPE_FWDREQ,
                                None
                            )
                            message = map(ord, header) + [command] + payload
                            success, message = gateway_stub.send(node_address, message)

                            if success: break
                            else: logger.error("RPC send got error replies: %s (retries %d-th time)" % (message, retries))
                        except:
                            logger.error("RPC send to ID=%s via gateway ID=%d addr=%s fails; error=%s\n%s" % (node_id,
                                gateway.id, str(gateway.tcp_address), str(e), traceback.format_exc()))
                        retries -= 1

                if retries == 0 or len(defer.allowed_replies) == 0:
                    logger.info("RPC send handler: returns immediately to handle failures, or defer has no expected replies")
                    defer.callback(None)

            gevent.sleep(0)
        #End of while True
    #End of def handler()



# Mock agent behavior, will only provide fixed responses and will not contact any external devices
# So no receive, and no call to any c library, no calls to broker, etc
class MockAgent(TransportAgent):
    _agent = None
    @classmethod
    def init(cls):
        if not cls._agent:
            cls._agent = MockAgent()
        return cls._agent

    def __init__(self):
        self._mode = 'stop'

        TransportAgent.__init__(self)

    # add a defer to queue
    def deferSend(self, destination, command, payload, allowed_replies, cb, error_cb):
        def callback(reply):
            cb(reply)

        def error_callback(reply):
            error_cb(reply)

        defer = new_defer(callback,
                error_callback,
                self.verify(allowed_replies),
                allowed_replies,
                new_message(destination, command, self.getNextSequenceNumberAsPrefixPayload() + payload),
                    int(round(time.time() * 1000)) + 10000)
        transport_agent_out_tasks.put_nowait(defer)
        return defer
    def getDeviceType(self,node):   #mock only wudevice, not sure what does the 3 fields do though, feel free to change it if you understand it
        return (None, 0xff, None)
    def send(self, destination, command, payload, allowed_replies):
        result = AsyncResult()

        def callback(reply):
            result.set(reply)

        def error_callback(reply):
            result.set(reply)


        defer = new_defer(callback,
                error_callback,
                self.verify(allowed_replies),
                allowed_replies,
                new_message(destination, command, self.getNextSequenceNumberAsPrefixPayload() + payload),
                    int(round(time.time() * 1000)) + 10000)
        transport_agent_out_tasks.put_nowait(defer)

        message = result.get() # blocking

        # received ack from Agent
        return message

    def routing(self):

        result = AsyncResult()

        def callback(reply):
            result.set(reply)

        defer = new_defer(callback,
                callback,
                None,
                None,
                new_message(1, "routing", 0),
                0)
        transport_agent_out_tasks.put_nowait(defer)

        return result.get()

    def discovery(self):
        return []

    def add(self):
        if self._mode != 'stop':
            return False
        return True

    def delete(self):
        if self._mode != 'stop':
            return False
        return True

    def stop(self):
        self._mode = 'stop'
        return True

    def poll(self):
        return "Not available"

    # to be run in a thread, and others will use ioloop to monitor this thread
    def handler(self):
        while 1:
            defer = transport_agent_out_tasks.get()

            if defer.message.command == "discovery":
                defer.callback(self.discovery())
            elif defer.message.command == "routing":
                defer.callback({})
            else:
                defer.callback(None)

class BrokerAgent:
    _broker_agent = None
    @classmethod
    def init(cls):
        if not cls._broker_agent:
            cls._broker_agent = BrokerAgent()
        return cls._broker_agent

    def __init__(self):
        #self.gid_server = GIDService()
        gevent.spawn(self.run)
        print '[transport] BrokerAgent init'

    def append(self, defer):
        getDeferredQueue().add_defer(defer)

    def run(self):
        while 1:
            # monitor pipes from receive
            deliver = broker_messages.get()
            print '[transport] getting message from nodes'
            print '[transport] ' + str(deliver)

            # display logs from nodes if received
            if deliver.command == pynvc.LOGGING:
                print '[transport] node %d : %s' % (deliver.destination,
                            str(bytearray(deliver.payload)))

            # find out which defer it is for
            defer_id, defer = getDeferredQueue().find_defer(deliver)

            if defer_id and defer:
                # call callback
                if deliver.command == pynvc.WKPF_ERROR_R:
                    defer.error_cb(deliver)
                else:
                    defer.callback(deliver)

                # remove it
                getDeferredQueue().remove_defer(defer_id)
            else:
                # if it is special message
                if not is_master_busy():
                    if deliver.command == pynvc.GROUP_NOTIFY_NODE_FAILURE:
                        print "[transport] reconfiguration message received"
                        wusignal.signal_reconfig()
                    else:
                        print "[transport] Failure: Unknown message received"
                else:
                    #log = "Incorrect reply received. Message type correct, but mptn_idnt pass verification: " + str(message)
                    print "[transport] message discarded"
                    print '[transport] ' + str(deliver)
            gevent.sleep(0)

Gateway = collections.namedtuple("Gateway", "id, tcp_address, if_address, if_address_len, prefix, prefix_len,\
    network, network_size, netmask, hostmask, uuid")
NodeInfo = collections.namedtuple("NodeInfo", "id, if_address, is_gateway, gateway_id, uuid")

class IDService:
    _id_server = None
    @classmethod
    def init(cls):
        if not cls._id_server:
            cls._id_server = IDService()
        return cls._id_server

    def __init__(self):
        self._is_joinable = False
        self._global_lock = RLock()
        self._nodes_db = MPTN.DBDict("master_node_info.sqlite")
        self._gateways_db = MPTN.DBDict("master_gateway_info.sqlite")
        self._init_lookup()

    def _init_lookup(self):
        with self._global_lock:
            self._nodes_lookup = {int(key):value for (key,value) in self._nodes_db.iteritems()}
            self._uuids_lookup = {node.uuid:node_id for (node_id, node) in self._nodes_lookup.iteritems()}
            self._gateways_lookup = {int(key):value for (key,value) in self._gateways_db.iteritems()}
            self._update_nexthop_hash()

            # _nexthop_lookup: key = network object genereated from an interface string "MPTN ID/NETMASK" STRING
            #                  value = NextHop namedtuple
            self._nexthop_lookup = {MPTN.ID_NETWORK_FROM_STRING("%s/%s"%(MPTN.ID_TO_STRING(int(gateway_id)),MPTN.ID_TO_STRING(gateway.netmask))):MPTN.NextHop(id=int(gateway_id),tcp_address=gateway.tcp_address) for (gateway_id, gateway) in self._gateways_lookup.iteritems()}

    def _update_nexthop_hash(self):
        self._nexthop_db = {"%s/%d"%(MPTN.ID_TO_STRING(int(gateway_id)),gateway.prefix_len):gateway.tcp_address for (gateway_id, gateway) in self._gateways_lookup.iteritems()}
        self._nexthop_db_json = "EmPtY"
        if len(self._nexthop_db) > 0:
            self._nexthop_db_json = json.dumps(self._nexthop_db, sort_keys=True)
        h = hashlib.sha512()
        h.update(self._nexthop_db_json)
        self._nexthop_hash = h.digest()

    def is_id_valid(self, mptn_id):
        return mptn_id in self._nodes_lookup

    def is_id_gateway(self, mptn_id):
        return mptn_id in self._gateways_lookup

    def start_accepting(self):
        with self._global_lock:
            self._is_joinable = True

    def stop_accepting(self):
        with self._global_lock:
            self._is_joinable = False

    def get_all_gateways(self):
        return self._gateways_lookup.itervalues()

    def get_gateway(self, mptn_id):
        if not self.is_id_valid(mptn_id):
            print "cannot find ID %d" % mptn_id
            return None

        if not self.is_id_gateway(mptn_id):
            mptn_id = self._nodes_lookup[mptn_id].gateway_id

        return self._gateways_lookup[mptn_id]

    def get_node_addr(self, mptn_id):
        node = self._nodes_lookup.get(mptn_id)
        if node is None: return None
        return node.if_address

    def _get_user_permission(self, to_check_id, gatetway_id, uuid):
        # TBD
        return True

    def _find_nexthop_for_id(self, mptn_id):
        if mptn_id is None or mptn_id == MPTN.MASTER_ID:
            return None

        for network, next_hop in self._nexthop_lookup.iteritems():
            if MPTN.IS_ID_IN_NETWORK(mptn_id, network):
                return next_hop
            gevent.sleep(0)

        return None

    def _add_new_node(self, node_info):
        self._nodes_db[node_info.id] = node_info
        self._nodes_lookup[node_info.id] = node_info
        self._uuids_lookup[node_info.uuid] = node_info.id

    def _del_old_node(self, mptn_id, uuid):
        node_id = self._uuids_lookup.pop(uuid)
        if mptn_id != node_id:
            logger.error("del_old_node ID unmatches")
            self._uuids_lookup[uuid] = node_id
            exit(-1)
        self._nodes_db.pop(node_id)
        self._nodes_lookup.pop(node_id)

    def _add_new_gateway(self, gateway_id, gateway):
        self._gateways_db[gateway_id] = gateway
        self._gateways_lookup[gateway_id] = gateway
        self._update_nexthop_hash()
        self._nexthop_lookup[gateway.network] = MPTN.NextHop(id=gateway_id,tcp_address=gateway.tcp_address)

    def _del_old_gateway(self, gateway_id):
        self._gateways_db.pop(gateway_id)
        gateway = self._gateways_lookup.pop(gateway_id)
        self._update_nexthop_hash()
        try:
            self._nexthop_lookup.pop(gateway.network)
        except Exception as e:
            logger.error("_del_old_gateway gateway_id %s %X occurs error:%s\n%s" % (MPTN.ID_TO_STRING(gateway_id), gateway_id, str(e), traceback.format_exc()))

    def _alloc_node_id(self, to_check_id, gateway_id, uuid):
        ERROR_ID = MPTN.MPTN_MAX_ID
        with self._global_lock:
            old_id = self._uuids_lookup.get(uuid)
            if old_id is not None:
                if old_id == ERROR_ID:
                    logger.error("IDREQ DB inconsistent: cannot insert an 0xFFFFFFFF into db")
                    exit(-1)

                if old_id != to_check_id:
                    logger.error("IDREQ odd things happened: both to-check ID %s 0x%X and old ID %s 0x%X has the same uuid=%s could be the same node or different nodes" % (
                        MPTN.ID_TO_STRING(to_check_id), to_check_id, MPTN.ID_TO_STRING(old_id), old_id, str(map(ord, uuid))))
                    return ERROR_ID

                else:
                    gateway = self.get_gateway(to_check_id)
                    if gateway is not None and gateway.id == gateway_id:
                        return to_check_id
                    else:
                        return ERROR_ID

            # old_id is None
            # It needs to allocate a new ID
            else:
                if not (self._is_joinable or ALLOW_MASTER_ALWAYS_JOINABLE):
                    logger.error("IDREQ cannot be handled at this time")
                    return ERROR_ID

                if to_check_id == ERROR_ID or self.is_id_valid(to_check_id):
                    logger.error("IDREQ to-check ID %s 0x%X cannot be a valid one or 0xFFFFFFFF" % (MPTN.ID_TO_STRING(to_check_id), to_check_id))
                    return ERROR_ID

                if self.is_id_gateway(to_check_id):
                    logger.error("IDREQ to-check ID %s 0x%X should not be a gateway" % (MPTN.ID_TO_STRING(to_check_id), to_check_id))
                    return ERROR_ID

                if not self._get_user_permission(to_check_id, gateway_id, uuid):
                    logger.error("IDREQ to-check ID %s 0x%X is not approved by user" % (MPTN.ID_TO_STRING(to_check_id), to_check_id))
                    return ERROR_ID

                gateway = self.get_gateway(gateway_id)
                if not MPTN.IS_ID_IN_NETWORK(to_check_id, gateway.network):
                    logger.error("IDREQ to-check ID %s 0x%X is not in gateway(ID %s 0x%X)'s network" % (MPTN.ID_TO_STRING(to_check_id), to_check_id, MPTN.ID_TO_STRING(gateway_id), gateway_id))
                    return ERROR_ID

                if_addr = (to_check_id & gateway.hostmask) if (gateway.if_address_len == 1 or gateway.if_address_len == 2) else to_check_id
                self._add_new_node(NodeInfo(id=to_check_id, is_gateway=False, gateway_id=gateway_id, uuid=uuid, if_address=if_addr))
                return to_check_id

    def _alloc_gateway_id(self, to_check_id, if_addr, if_addr_len, if_netmask, ip, port, uuid):
        ERROR_ID = MPTN.MPTN_MAX_ID

        with self._global_lock:
            old_id = self._uuids_lookup.get(uuid)
            if old_id is not None:
                if old_id == ERROR_ID:
                    logger.error("GWIDREQ inconsistent: cannot insert an 0xFFFFFFFF into db")
                    exit(-1)

                if to_check_id != ERROR_ID and old_id != to_check_id:
                    logger.error("GWIDREQ odd things happened: both to-check ID %s 0x%X and old ID %s 0x%X has the same uuid=%s could be the same gateway or different gateways" % (MPTN.ID_TO_STRING(to_check_id), to_check_id,
                        MPTN.ID_TO_STRING(old_id), old_id, str(map(ord, uuid))))
                    return ERROR_ID

                # It's a  Got an old ID to check
                else:
                    if to_check_id == ERROR_ID or (self.is_id_valid(to_check_id) and not self.is_id_gateway(to_check_id)):
                        logger.error("GWIDREQ got a invalid tocheck ID")
                        return ERROR_ID

                    gateway = self.get_gateway(to_check_id)
                    if gateway.uuid != uuid:
                        logger.error("GWIDREQ uuid unmatched")
                        return ERROR_ID

                    if gateway.tcp_address[0] != ip or gateway.tcp_address[1] != port:
                        logger.error("GWIDREQ tcp_address unmatched")
                        return ERROR_ID

                    if gateway.if_address != if_addr:
                        logger.error("GWIDREQ if_addr unmatched")
                        return ERROR_ID

                    if gateway.if_address_len != if_addr_len:
                        logger.error("GWIDREQ if_address_len unmatched")
                        return ERROR_ID

                    if gateway.netmask != if_netmask:
                        logger.error("GWIDREQ netmask unmatched")
                        return ERROR_ID

                    return to_check_id

            # old_id is None
            # It needs to allocate a new ID
            else:
                """
                Gateway = collections.namedtuple("Gateway", "id, tcp_address, if_address, if_address_len, prefix, prefix_len,\
                    network, network_size, netmask, hostmask, uuid")
                """
                if if_addr_len > MPTN.MPTN_ID_LEN:
                    logger.error("GWIDREQ gateway if_addr_len is too long to support: %d bytes" % if_addr_len)
                    return ERROR_ID

                elif if_addr_len == 2 or if_addr_len == 1:
                    if if_addr >= 2 ** (if_addr_len*8):
                        logger.error("GWIDREQ gateway if_addr %d >= network_size %d" % (if_addr, network_size))
                        return ERROR_ID

                    super_net = "%s/%d" % (MPTN.ID_TO_STRING(if_addr), (MPTN.MPTN_ID_LEN-if_addr_len)*8) # "0.0.0.0/24" for ZW, "0.0.0.0/16" for ZB
                    subnet_candidates = MPTN.ID_NETWORK_FROM_STRING(super_net).supernet(prefixlen_diff=8).subnets(prefixlen_diff=8)
                elif if_addr_len == 4:

                    super_net = "%s/%s" % (MPTN.ID_TO_STRING(if_addr), MPTN.ID_TO_STRING(if_netmask))
                    subnet_candidates = [MPTN.ID_NETWORK_FROM_STRING(super_net)]
                else:
                    logger.error("GWIDREQ unsupported gateway if_addr_len %d" % (if_addr_len))
                    return ERROR_ID

                for gateway in self.get_all_gateways():
                    if gateway.tcp_address == (ip, port):
                        logger.error("GWIDREQ found duplicate TCP Address %s, delete old uuid %s" % (str((ip, port)), str(map(ord, gateway.uuid))))
                        self._del_old_gateway(gateway.id)
                        self._del_old_node(gateway.id, gateway.uuid)
                        break

                interface = MPTN.ID_INTERFACE_FROM_TUPLE(MPTN.ID_TO_STRING(if_addr), MPTN.ID_TO_STRING(if_netmask))
                network_size = interface.network.num_addresses
                hostmask = int(interface.network.hostmask)
                prefix_len = interface.network.prefixlen
                netmask = int(interface.network.netmask)

                network = None
                for subnet in subnet_candidates:
                    for gateway in self.get_all_gateways():
                        if MPTN.IS_ID_IN_NETWORK(gateway.id, subnet):
                            break
                        gevent.sleep(0)
                    else:
                        # it's a unused one
                        prefix = int(subnet.network_address)
                        network = subnet
                        break
                    gevent.sleep(0)
                else:
                    logger.error("GWIDREQ cannot find free gateway ID for if_addr=%s if_addr_len=%d if_netmask=%s ip=%s port=%d uuid=%s" % (ID_TO_STRING(if_addr), if_addr_len, ID_TO_STRING(if_netmask). ip, port, str(map(ord, uuid))))
                    return ERROR_ID

                new_id = prefix | if_addr
                gateway = Gateway(id=new_id, tcp_address=(ip, port),
                    if_address=if_addr, if_address_len=if_addr_len,
                    prefix=prefix, prefix_len=prefix_len,
                    network=network, network_size=network_size,
                    netmask=netmask, hostmask=hostmask,
                    uuid=uuid)

                self._add_new_gateway(new_id, gateway)

                node = NodeInfo(id=new_id, is_gateway=True, gateway_id=new_id, uuid=uuid, if_address=if_addr)
                self._add_new_node(node)
                return new_id

    def handle_gwidreq_message(self, context, dest_id, src_id, msg_type, payload):
        message = MPTN.create_packet_to_str(src_id, dest_id, MPTN.MPTN_MSGTYPE_GWIDNAK, None)
        if dest_id != MPTN.MASTER_ID:
            logger.error("GWIDREQ dest id 0x%X is not Master" % (dest_id))
            MPTN.socket_send(context, MPTN.MPTN_MAX_ID, message)
            return

        if src_id != MPTN.MPTN_MAX_ID and not self.is_id_gateway(src_id):
            logger.error("GWIDREQ src id 0x%X is not valid" % (src_id))
            MPTN.socket_send(context, MPTN.MPTN_MAX_ID, message)
            return

        if payload is None:
            logger.error("GWIDREQ might have the payload")
            MPTN.socket_send(context, MPTN.MPTN_MAX_ID, message)
            return

        ip = context.address[0]

        # m = re.match("IFADDR=(\d+);IFADDRLEN=(\d+);IFNETMASK=(\d+);PORT=(\d+);UUID=(.+)",payload)
        # if m is None or len(m.group(5)) != MPTN.GWIDREQ_PAYLOAD_LEN:
        #     logger.error("GWIDREQ payload should be IFADDR=int;IFADDRLEN=int;IFNETMASK=int;PORT=int;UUID=16bytes")
        #     MPTN.socket_send(context, 0xFFFFFFFF, message)
        #     return

        # if_addr = int(m.group(1))
        # if_addr_len = int(m.group(2))
        # if_netmask = int(m.group(3))
        # port = int(m.group(4))
        # uuid = m.group(5)

        payload = json.loads(payload)
        if_addr = payload["IFADDR"]
        if_addr_len = payload["IFADDRLEN"]
        if_netmask = payload["IFNETMASK"]
        port = payload["PORT"]
        try:
            uuid = struct.pack("!%dB"%MPTN.GWIDREQ_PAYLOAD_LEN, *payload["UUID"])
        except Exception as e:
            logger.error("GWIDREQ payload should be a json 'IFADDR':int, 'IFADDRLEN':int, 'IFNETMASK':int, 'PORT':int, 'UUID':list with 16 unsigned chars; error=%s\n%s" % (str(e), traceback.format_exc()))
            MPTN.socket_send(context, MPTN.MPTN_MAX_ID, message)
            return

        new_id = self._alloc_gateway_id(src_id, if_addr, if_addr_len, if_netmask, ip, port, uuid)
        msg_type = MPTN.MPTN_MSGTYPE_GWIDACK if new_id != MPTN.MPTN_MAX_ID else MPTN.MPTN_MSGTYPE_GWIDNAK
        message = MPTN.create_packet_to_str(new_id, MPTN.MASTER_ID, msg_type, None)
        context.id = new_id
        MPTN.socket_send(context, new_id, message)
        logger.info("GWIDREQ new ID %s 0x%X is ACK-ed with details: if_addr=%s, if_addr_len=%d, if_netmask=%s, ip=%s, port=%d" % (MPTN.ID_TO_STRING(new_id), new_id, MPTN.ID_TO_STRING(if_addr), if_addr_len, MPTN.ID_TO_STRING(if_netmask), MPTN.ID_TO_STRING(ip), port))
        return

    def handle_idreq_message(self, context, dest_id, src_id, msg_type, payload):
        gateway_id = dest_id
        to_check_id = src_id

        message = MPTN.create_packet_to_str(src_id, dest_id, MPTN.MPTN_MSGTYPE_IDNAK, None)
        if not self.is_id_gateway(gateway_id):
            logger.error("IDREQ with unknown gateway ID 0x%X" % gateway_id)
            MPTN.socket_send(context, MPTN.MPTN_MAX_ID, message)
            return

        if payload is None or len(payload) != MPTN.IDREQ_PAYLOAD_LEN:
            logger.error("IDREQ might have a %d-byte payload" % MPTN.IDREQ_PAYLOAD_LEN)
            MPTN.socket_send(context, MPTN.MPTN_MAX_ID, message)
            return

        new_id = self._alloc_node_id(to_check_id, gateway_id, payload)
        msg_type = MPTN.MPTN_MSGTYPE_IDACK if new_id != MPTN.MPTN_MAX_ID else MPTN.MPTN_MSGTYPE_IDNAK
        message = MPTN.create_packet_to_str(new_id, MPTN.MASTER_ID, msg_type, None)
        context.id = new_id
        MPTN.socket_send(context, new_id, message)

        if msg_type == MPTN.MPTN_MSGTYPE_IDACK:
            logger.info("IDREQ new ID %s 0x%X is ACK-ed with details: gateway_id=%s" % (MPTN.ID_TO_STRING(new_id), new_id, MPTN.ID_TO_STRING(gateway_id)))
        else:
            logger.info("IDREQ tocheck ID %s 0x%X is NAK-ed with details: gateway_id=%s" % (MPTN.ID_TO_STRING(to_check_id), to_check_id, MPTN.ID_TO_STRING(gateway_id)))

    def handle_fwdreq_message(self, context, dest_id, src_id, msg_type, payload):
        msg_type = MPTN.MPTN_MSGTYPE_FWDACK

        if not self.is_id_valid(src_id):
            logger.error("invalid FWDREQ src ID %X %s: not found in network" % (src_id, MPTN.ID_TO_STRING(src_id)))
            msg_type = MPTN.MPTN_MSGTYPE_FWDNAK

        if dest_id != MPTN.MASTER_ID:
            logger.error("invalid FWDREQ dest_id %X %s: not Master" % (src_id, MPTN.ID_TO_STRING(dest_id)))
            msg_type = MPTN.MPTN_MSGTYPE_FWDNAK

        if payload is None:
            logger.error("FWDREQ should have payload")
            msg_type = MPTN.MPTN_MSGTYPE_FWDNAK

        message = MPTN.create_packet_to_str(src_id, dest_id, msg_type, payload)
        MPTN.socket_send(context, src_id, message)

        if msg_type == MPTN.MPTN_MSGTYPE_FWDACK:
            payload = map(ord, payload)
            broker_messages.put_nowait(new_deliver(src_id, payload[0], payload[1:]))
        return

    def handle_rtping_message(self, context, dest_id, src_id, msg_type, payload):
        if not self.is_id_valid(src_id):
            logger.error("invalid RTPING src ID %X %s: not found in network" % (src_id, MPTN.ID_TO_STRING(src_id)))
            return

        if dest_id != MPTN.MASTER_ID:
            logger.error("invalid RTPING dest_id %X %s: not Master" % (src_id, MPTN.ID_TO_STRING(dest_id)))
            return

        if payload is None:
            logger.error("RTPING should have payload")
            return

        if payload == self._nexthop_hash:
            # logger.debug("RTPING got the same hash. no need reply")
            return

        message = MPTN.create_packet_to_str(src_id, dest_id, MPTN.MPTN_MSGTYPE_RTPING, self._nexthop_hash)
        # logger.info("new hash is %s" % str(map(ord, self._nexthop_hash)))
        # logger.info("db %s lookup %s" % (str(self._nexthop_db), str(self._nexthop_lookup)))
        MPTN.socket_send(None, src_id, message)
        return

    def handle_rtreq_message(self, context, dest_id, src_id, msg_type, payload):
        if not self.is_id_valid(src_id):
            logger.error("invalid RTREQ src ID %X %s: not found in network" % (src_id, MPTN.ID_TO_STRING(src_id)))
            return

        if dest_id != MPTN.MASTER_ID:
            logger.error("invalid RTREQ dest_id %X %s: not Master" % (src_id, MPTN.ID_TO_STRING(dest_id)))
            return

        if payload is not None:
            logger.error("RTREQ should not have payload")
            return

        message = MPTN.create_packet_to_str(src_id, dest_id, MPTN.MPTN_MSGTYPE_RTREP, self._nexthop_db_json)
        MPTN.socket_send(None, src_id, message)
        return

def getMockAgent():
    return MockAgent.init()

def getAgent():
    return BrokerAgent.init()

def getGatewayAgent():
    return RPCAgent.init()

def getNetworkServerAgent():
    return NetworkServerAgent.init()

def getDeferredQueue():
    return DeferredQueue.init()

def getIDService():
    return IDService.init()