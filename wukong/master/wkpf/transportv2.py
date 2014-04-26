import sys, os, fcntl
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

import tornado.ioloop
import hashlib
import logging
import collections
import gevent
from gevent.event import AsyncResult
import gevent.queue
import wusignal
from configuration import *
from globals import *
import pynvc

import struct
from gevent import socket
from tinyrpc.protocols.jsonrpc import JSONRPCProtocol
from tinyrpc.transports import ClientTransport
from tinyrpc import RPCClient
import mptnUtils
import ipaddress
from gevent.lock import RLock

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
                print "[transport] Either one of " + str(defer.allowed_replies) + " expected from defer " + str(defer) + " does not match or the sequence number got skewed: " + str(deliver)
        print '[transport] not found'
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
A tasks queue and a messages queue
'''
tasks = gevent.queue.Queue()
messages = gevent.queue.Queue()


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
    def __init__(self, server_did, server_ip, server_port):
        self.server_did = server_did
        self.server_ip = server_ip
        self.server_port = server_port

    def _process_message(self, message):
        dest_did, src_did, msg_type, msg_subtype = mptnUtils.extract_mult_proto_header_from_str(message)

        if len(message) <= mptnUtils.MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET:
            print "[transport] RPC drops message since payload is empty"
            return None

        payload = message[mptnUtils.MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET:]
        
        if dest_did != getDIDService().get_master_did():
            print "[transport] RPC drops message since DEST DID (%X) is not master" % dest_did
            return None

        if not getDIDService().is_did_valid(src_did):
            print "[transport] RPC drops message since SRC DID (%X) is not valid" % src_did
            return None

        if msg_type != mptnUtils.MULT_PROTO_MSG_TYPE_RPC:
            print "[transport] RPC drops message since msg_type (%X) is not RPC" % msg_type
            return None

        if msg_subtype != mptnUtils.MULT_PROTO_MSG_SUBTYPE_RPC_REP:
            print "[transport] RPC drops message since msg_type (%X) is not RPC" % msg_type
            return None

        return payload

    def send_message(self, message, expect_reply=True):
        address = (self.server_ip, self.server_port)
        for i in xrange(mptnUtils.CONNECTION_RETRIES):
            try:
                sock = socket.create_connection(address, mptnUtils.NETWORK_TIMEOUT)
                break
            except IOError as e:
                print '[transport] RPC failed to connect to master %s:%s: %s', address[0], address[1], str(e)
        else:
            if sock is not None: sock.close()
            return None

        ret = None
        try:
            #sock.settimeout(mptnUtils.NETWORK_TIMEOUT)

            mptnUtils.special_send(sock, message)

            if expect_reply:
                message = mptnUtils.special_recv(sock)
                ret = self._process_message(message)

        except socket.timeout:
            print "[transport] RPC socket is timeout with addr=%s with msg=%s" % (str(address), message)
        except socket.error as e:
            print"[transport] RPC gets socket error %s with addr=%s with msg=%s" % (str(e), str(address), message)
        except struct.error as e:
            print "[transport] RPC python struct cannot interpret message %s with error %s" % (message, str(e))
        finally:
            sock.close()
        
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
        TransportAgent.__init__(self)

    def _create_client_stub(self, did, ip, port):
        assert isinstance(did, int), "did should be integer"
        assert isinstance(ip, basestring), "port should be string"
        assert isinstance(port, int), "port should be integer"
        rpc_client = RPCClient(
                JSONRPCProtocol(),
                RPCTCPClient(did, ip, port)
            )
        return rpc_client.get_proxy() # Usage: ret_obj.func()

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

        tasks.put_nowait(defer)
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
                int(round(time.time() * 1000)) + 10000
        )

        tasks.put_nowait(defer)

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

        tasks.put_nowait(defer)

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

        tasks.put_nowait(defer)

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

        tasks.put_nowait(defer)

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
            getDIDService().start_accepting()
        else:
            getDIDService().stop_accepting()

        count = 0
        for did, ip, port in getDIDService().get_all_gateways():
            gateway = self._create_client_stub(did, ip, port)
            try:
                if to_mode == 'add':
                    if gateway.add():
                        count += 1
                elif to_mode == 'delete': 
                    if gateway.delete():
                        count += 1
                else: 
                    if gateway.stop():
                        count += 1
            except:
                print "[transport] RPC call %s() to gateway (%d %s) fails" % (to_mode, did, str((ip, port)))
        
        if count > 0:
            self._mode = to_mode
            return True

        print "[transport] RPC calls %s() to all gateways fail or no gateway found" % to_mode
        getDIDService().stop_accepting()
        return False

    def add(self):
        return self._learn('add')

    def delete(self):
        return self._learn('delete')

    def stop(self):
        return self._learn('stop')

    def poll(self):
        ret = []
        for did, ip, port in getDIDService().get_all_gateways():
            gateway = self._create_client_stub(did, ip, port)
            try:
                p = gateway.poll()
                ret.append(p)
            except:
                print "[transport] RPC poll() to gateway(%d %s) fails" % (did, str((ip, port)))
        
        if len(ret):
            return ret.join("; ")
        
        print "[transport] RPC poll() to gateway(%d %s) fail or no gateway found" % (did, str((ip, port)))
        return "Not available"

    def _process_message(self, context, message):
        if len(message) < mptnUtils.MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET:
            print "[transport] TCP Server receives message with header shorter than required %s" % mptnUtils.print_packet(str(message))
            return None, None

        dest_did, src_did, msg_type, msg_subtype = mptnUtils.extract_mult_proto_header_from_str(message)
        payload = message[mptnUtils.MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET:] if len(message) > mptnUtils.MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET else None

        assert isinstance(context, tuple), "[transport] TCP Server context should be an IP address tuple"
        context_str = ':'.join([str(i) for i in context])
        log_msg = [context_str] + mptnUtils.split_packet_header(message)
        if payload is not None: log_msg.append(payload)
        log_msg = mptnUtils.formatted_print(log_msg)
        print "[transport] TCP Server receives message:\n" + log_msg

        if msg_type == mptnUtils.MULT_PROTO_MSG_TYPE_PFX:
            # Only PFX REQ message from gateway would get here
            if msg_subtype != mptnUtils.MULT_PROTO_MSG_SUBTYPE_PFX_REQ:
                print "[transport] TCP Server receives message subtype other than PFX UPD and will not send any reply"
                return None, None

            if dest_did != getDIDService().get_master_did() and src_did != 0xFFFFFFFF:
                print "[transport] TCP server: PFX REQ with incorrect src did = %X and/or dest did = %X" % (src_did, dest_did)
                return None, None

            if payload is None:
                print "[transport] TCP server: PFX REQ might have the payload"
                return None, None
            
            # _pfx_req_handler "RADDR=%d;LEN=%d;PORT=%d"
            payload = payload.split(";")
            if len(payload) != 3:
                print "[transport] TCP server: PFX REQ payload should be RADDR=int;LEN=int;PORT=int"
                return None, None
            
            print "[transport] TCP Server receives a PFX REQ message"

            try:
                raddr = int(payload[0].split("=")[1])
                raddr_len = int(payload[1].split("=")[1])
                port = int(payload[2].split("=")[1])
            except:
                print "[transport] TCP server: PFX REQ payload should be RADDR=int;LEN=int;PORT=int"
                return None, None

            dest_did = getDIDService().get_or_allocate_gateway_did(raddr, raddr_len, context[0], port)
            msg_subtype = mptnUtils.MULT_PROTO_MSG_SUBTYPE_PFX_ACK if dest_did != 0xFFFFFFFF else mptnUtils.MULT_PROTO_MSG_SUBTYPE_PFX_NAK
            message = mptnUtils.create_mult_proto_header_to_str(dest_did, getDIDService().get_master_did(), msg_type, msg_subtype)

            log_msg = mptnUtils.split_packet_header(message)
            log_msg = mptnUtils.formatted_print(log_msg)
            if msg_subtype == mptnUtils.MULT_PROTO_MSG_SUBTYPE_PFX_ACK:
                print "[transport] TCP Server replies PFX ACK message:\n" + log_msg
            else:
                print "[transport] TCP Server replies PFX NAK message:\n" + log_msg

            return (1, message)

        elif msg_type == mptnUtils.MULT_PROTO_MSG_TYPE_DID:
            # Only DID UPD message from gateway would get here
            if msg_subtype != mptnUtils.MULT_PROTO_MSG_SUBTYPE_DID_UPD:
                print "[transport] TCP server receives invalid DID message subtype %X and will not send any reply" % msg_subtype
                return None, None

            if not getDIDService().is_did_valid(dest_did):
                print "[transport] TCP server: DID UPD with incorrect src did = %X and/or dest did = %X" % (src_did, dest_did)
                return None, None

            if payload is None:
                print "[transport] TCP server: DID UPD might have the payload"
                return None, None

            if len(payload) != 8:
                print "[transport] TCP server: DID UPD payload must be 8 bytes (MAC address)"
                return None, None

            print "[transport] TCP Server receives a DID UPD message"

            # _did_req_handler
            registering_did = src_did
            gateway_did = dest_did
            mac_addr = payload
            registered_did = getDIDService().allocate_device_did(registering_did, gateway_did, mac_addr)
            msg_subtype = mptnUtils.MULT_PROTO_MSG_SUBTYPE_DID_ACK if registered_did != 0xFFFFFFFF else mptnUtils.MULT_PROTO_MSG_SUBTYPE_DID_NAK
            message = mptnUtils.create_mult_proto_header_to_str(registered_did, getDIDService().get_master_did(), msg_type, msg_subtype)

            log_msg = mptnUtils.split_packet_header(message)
            log_msg = mptnUtils.formatted_print(log_msg)
            if msg_subtype == mptnUtils.MULT_PROTO_MSG_SUBTYPE_DID_ACK:
                print "[transport] TCP Server replies DID ACK message:\n" + log_msg
            else:
                print "[transport] TCP Server replies DID NAK message:\n" + log_msg

            return (1, message)

        # For the rest of msg_type
        # Check if SRC_DID is on MPTN network
        # Also check if DEST_DID is on MPTN network
        # Drop if one of test is not passed
        if dest_did != getDIDService().get_master_did():
            print "[transport] TCP server drops message since DEST DID (%X) is not master" % dest_did
            return None, None

        if not getDIDService().is_did_valid(src_did):
            print "[transport] TCP server drops message since SRC DID (%X) is not valid" % src_did
            return None, None        
        
        if msg_type == mptnUtils.MULT_PROTO_MSG_TYPE_FWD:
            # Check if DEST_DID is on the same network and get radio address of it
            #   True:   send to transport and return back to source gateway or master
            #   False:  find and then send to radio address of destination IP hop
            if payload is None:
                print "[transport] TCP server: packet FWD might have the payload"
                return None, None

            if msg_subtype != mptnUtils.MULT_PROTO_MSG_SUBTYPE_FWD:
                print "[transport] TCP server receives invalid FWD FWD message subtype and will not send any reply"
                return None, None 
            
            print "[transport] TCP Server receives a FWD message"

            msg_subtype = mptnUtils.MULT_PROTO_MSG_SUBTYPE_FWD_ACK
            header = mptnUtils.create_mult_proto_header_to_str(src_did, dest_did, msg_type, msg_subtype)

            return (2, (header, src_did, payload))

        else:
            print "[transport] TCP server receives unknown message with type %X" % msg_type
            return None, None 

    def receive(self, timeout_msec=100):
        try:
            server = socket.socket()
            prevent_socket_inheritance(server)
            server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            server.bind(('', 9010))
            server.listen(500)
        except socket.error as msg:
            server.close()
            print "[transport] TCP Server cannot start due to socket error: %s" % msg
            return

        def handle_socket(sock, addr):
            try:
                print "[transport] TCP Server accepts connection from address %s" % str(addr)
                sock.settimeout(mptnUtils.NETWORK_TIMEOUT)
                message = mptnUtils.special_recv(sock)
                status, response = self._process_message(addr, message)
                if status is None:
                    return

                if status == 1:
                    mptnUtils.special_send(sock, response)

                elif status == 2:
                    mptnUtils.special_send(sock, response[0])
                    src = response[1]
                    reply = response[2]
                    if src and reply:
                        deliver = new_deliver(src, reply[0], reply[1:])
                        messages.put_nowait(deliver)
                        print '[transport] TCP Server receive: put a message to messages'

            except socket.timeout:
                print "[transport] TCP Server socket is timeout from addr=%s with msg=%s" % (str(addr),message)
            except socket.error as e:
                print "[transport] TCP Server gets socket error %s from addr=%s with msg=%s" % (str(e), str(addr), message)
            except struct.error as e:
                print "[transport] TCP Server python struct cannot interpret message %s" % message
            finally:
                sock.close()
            getDeferredQueue().removeTimeoutDefer()

        print "[transport] TCP server for MPTN is listening on %s" % str(server.getsockname())
        while True:
            new_sock, address = server.accept()
            prevent_socket_inheritance(new_sock)
            gevent.spawn(handle_socket, new_sock, address)
            gevent.sleep(0.01) 

    def handler(self):
        while True:
            defer = tasks.get()
            #print 'handler: getting defer from task queue'

            if defer.message.command == "discovery":
                #print 'handler: processing discovery request'
                discovered_nodes = []
                for did, ip, port in getDIDService().get_all_gateways():
                    gateway = self._create_client_stub(did, ip, port)
                    try:
                        d = gateway.discover()
                        max_num = 2**(getDIDService().get_gateway_raddr_len(did)*8)-1
                        prefix = getDIDService().get_gateway_prefix(did)
                        for i in xrange(len(d)):
                            if d[i] > max_num:
                                print "[transport] RPC discover() to gateway(%d %s) fails due to incorrect radio address %d" % (did, str((ip, port)), d[i])
                                d = []
                                break
                            d[i] = prefix | d[i]
                        discovered_nodes += d
                    except:
                        print "[transport] RPC discover() to gateway(%d %s) fails" % (did, str((ip, port)))
                defer.callback(discovered_nodes)

            elif defer.message.command == "routing":
                #print 'handler: processing routing request'
                routing = {}
                for did, ip, port in getDIDService().get_all_gateways():
                    gateway = self._create_client_stub(did, ip, port)
                    try:
                        r = gateway.routing()
                        max_num = 2**(getDIDService().get_gateway_raddr_len(did)*8)-1
                        prefix = getDIDService().get_gateway_prefix(did)
                        for rnode, rlist in r.items():
                            if rnode > max_num:
                                break
                            rnode = prefix | rnode
                            for i in xrange(len(rlist)):
                                if rlist[i] > max_num:
                                    print "[transport] RPC routing() to gateway(%d %s) fails due to incorrect radio address %d in the routing list of device DID %d" % (did, str((ip, port)), rlist[i], rnode)
                                    rlist = []
                                    break
                                rlist[i] = prefix | rlist[i]
                            if len(rlist) == 0:
                                break
                            routing[rnode] = rlist
                    except:
                        print "[transport] RPC routing() to gateway(%s) fails" % gw_addr

                defer.callback(routing)

            elif defer.message.command == "device_type":
                device_type = None
                dev_did = defer.message.destination
                dev_addr = getDIDService().get_device_raddr(dev_did)
                if dev_addr is not None:
                    gw_did, gw_ip, gw_port = getDIDService().get_gateway(dev_did)
                    if gw_did is not None:
                        gateway = self._create_client_stub(gw_did, gw_ip, gw_port)
                        try:
                            device_type = gateway.getDeviceType(dev_addr)
                        except:
                            print "[transport] RPC getDeviceType() to DID(%d) and gateway(%d %s) fails" % (dev_did, gw_did, str((gw_ip, gw_port)))

                defer.callback(device_type)

            else: # defer.message.command is 'send'
                retries = 1
                dev_did = defer.message.destination
                command = defer.message.command
                payload = defer.message.payload

                dev_addr = getDIDService().get_device_raddr(dev_did)
                if dev_addr is None:
                    retries = 0
                else:
                    gw_did, gw_ip, gw_port = getDIDService().get_gateway(dev_did)

                    if gw_did is None:
                        retries = 0
                    else:
                        gateway = self._create_client_stub(gw_did, gw_ip, gw_port)

                        # prevent pyzwave send got preempted and defer is not in queue
                        if len(defer.allowed_replies) > 0:
                            print "[transport] handler: appending defer %s to queue" % str(defer)
                            getAgent().append(defer)

                        while retries > 0:
                            try:
                                #print "handler: sending message from defer"
                                message = [0x88, command] + payload
                                message = getDIDService().create_fwd_message_byte_list(dev_did, message)

                                success, msg = gateway.send(dev_addr, message)

                                if success: break

                                print "[transport] RPC send() replies %s so retries %d time(s)" % (msg, retries)
                            except:
                                print "[transport] RPC send() to DID(%d) and gateway(%d %s) fails" % (dev_did, gw_did, str((gw_ip, gw_port)))

                if retries == 0 or len(defer.allowed_replies) == 0:
                    print "[transport] handler: returns immediately to handle failures, or defer has no expected replies"
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
                new_message(destination, command, self.getNextSequenceNumberAsPrefixPayload() + payload), int(round(time.time() * 1000)) + 10000)
        tasks.put_nowait(defer)
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
                new_message(destination, command, self.getNextSequenceNumberAsPrefixPayload() + payload), int(round(time.time() * 1000)) + 10000)
        tasks.put_nowait(defer)

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
        tasks.put_nowait(defer)

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
            defer = tasks.get()

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
            deliver = messages.get()
            print '[transport] getting messages from nodes'
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
                # if it is special messages
                if not is_master_busy():
                    if deliver.command == pynvc.GROUP_NOTIFY_NODE_FAILURE:
                        print "[transport] reconfiguration message received"
                        wusignal.signal_reconfig()
                    else:
                        print "[transport] Failure: Unknown message received"
                else:
                    #log = "Incorrect reply received. Message type correct, but didnt pass verification: " + str(message)
                    print "[transport] message discarded"
                    print '[transport] ' + str(deliver)
            gevent.sleep(0)

class DIDService:
    _did_server = None
    @classmethod
    def init(cls):
        if not cls._did_server:
            cls._did_server = DIDService()
        return cls._did_server

    def __init__(self):
        self._device_dids = mptnUtils.DBDict("master_dev_dids.sqlite")
        self._gateway_dids = mptnUtils.DBDict("master_gtw_dids.sqlite")
        self._device_macs = mptnUtils.DBDict("master_dev_macs.sqlite")
        self._is_insertable = False
        self._global_lock = RLock()

    def _update_gateways(self, new_did, new_ip, new_port, new_mask):
        if len(self._gateway_dids) < 2:
            return
        # update other gateway PFX UPD
        to_new_gateway_str = []
        new_did_str = str(ipaddress.ip_address(new_did))
        for did, ip, port in self.get_all_gateways():
            # payload "DID/MASK=IP:PORT"
            did_str = str(ipaddress.ip_address(did))
            mask = self.get_gateway_prefix_len(did)
            if did == new_did:
                to_new_gateway_str.append("%s/%d=%s:%d" % (did_str, mask, ip, port))
            else:
                header = mptnUtils.create_mult_proto_header_to_str(did, getDIDService().get_master_did(), mptnUtils.MULT_PROTO_MSG_TYPE_PFX, mptnUtils.MULT_PROTO_MSG_SUBTYPE_PFX_UPD)
                payload = "%s/%d=%s:%d" % (new_did_str, new_mask, new_ip, new_port)
                self._send_to((ip, port), header+payload)

        header = mptnUtils.create_mult_proto_header_to_str(new_did, getDIDService().get_master_did(), mptnUtils.MULT_PROTO_MSG_TYPE_PFX, mptnUtils.MULT_PROTO_MSG_SUBTYPE_PFX_UPD)
        payload = ";".join(to_new_gateway_str)
        self._send_to((ip, port), header+payload)

    def _send_to(self, address, message):
        for i in xrange(mptnUtils.CONNECTION_RETRIES):
            try:
                sock = socket.create_connection(address, mptnUtils.NETWORK_TIMEOUT)
                break
            except IOError as e:
                print '[transport] failed to connect to gateway %s:%s: %s', address[0], address[1], str(e)
        else:
            if sock is not None: sock.close()
            return None

        try:
            #sock.settimeout(mptnUtils.NETWORK_TIMEOUT)
            mptnUtils.special_send(sock, message)

        except socket.timeout:
            print "[transport] gateway socket is timeout with addr=%s with msg=%s" % (str(address), message)
        except socket.error as e:
            print"[transport] gets socket error %s to gateway with addr=%s with msg=%s" % (str(e), str(address), message)
        except struct.error as e:
            print "[transport] python struct cannot interpret message %s to gateway with error %s" % (message, str(e))
        finally:
            sock.close()
    
    def get_master_did(self):
        return 0

    def is_did_valid(self, did):
        if did in self._device_dids:
            return True

        if did in self._gateway_dids:
            return True

    def start_accepting(self):
        with self._global_lock:
            self._is_insertable = True

    def stop_accepting(self):
        with self._global_lock:
            self._is_insertable = False

    def allocate_device_did(self, did, gateway_did, mac_addr=None):
        if mac_addr is not None and mac_addr in self._device_macs:
            if self._device_macs[mac_addr] != did:
                return 0xFFFFFFFF
        
        with self._global_lock:
            self._device_macs[mac_addr] = did
            prefix_len = self.get_gateway_prefix_len(gateway_did)
            raddr = (did << prefix_len) >> prefix_len
            self._device_dids[did] = (gateway_did, raddr)
        return did

    def get_or_allocate_gateway_did(self, g_raddr, g_raddr_len, g_ip, g_port):
        error_did = 0xFFFFFFFF
        for did, (ip, port, prefix, prefix_len, raddr, raddr_len) in self._gateway_dids.iteritems():
            did = int(did)
            if g_ip == ip and g_port == port:
                if g_raddr_len != raddr_len or g_raddr != raddr:
                    print "[DID] there is a gateway's DID %d(%s) matching the ip %s and port %d instead of the radio address %d (?=%d) or its length %d (?=%d)" %(did, str(ipaddress.ip_address(did)), ip, port, g_addr, raddr, g_raddr, raddr_len, g_raddr_len)
                    return error_did
                print "[DID] one gateway's DID %d(%s) found to match the ip %s and port %d" %(did, str(ipaddress.ip_address(did)), ip, port)
                return did
        else:
            max_len = mptnUtils.MULT_PROTO_LEN_DID
            prefix_len = (max_len-g_raddr_len)*8

            if g_raddr_len > max_len:
                print "[DID] gateway's radio address length exceeds %d bytes" % max_len
                return error_did

            if g_raddr > 2**(g_raddr_len*8):
                print "[DID] gateway's radio address %d exceeds %d" % (g_raddr, 2**(g_raddr_len*8))
                return error_did

            if g_raddr_len == 4:
                raise NotImplementedError
            elif g_raddr_len == 1:
                candidates = ipaddress.ip_network("0.0.0.0/16").subnets(prefixlen_diff=8)
            elif g_raddr_len == 2:
                candidates = ipaddress.ip_network("0.0.0.0/8").subnets(prefixlen_diff=8)
            else:
                return error_did

            for c in candidates:
                for did in self._gateway_dids:
                    did = int(did)
                    if ipaddress.ip_address(did) in c:
                        break
                else:
                    prefix = int(c.network_address)
                    break
            else:
                print "[DID] cannot find any available gateway DID for the ip %s and port %d" %(ip, port)
                return error_did
                    
            did = prefix | g_raddr
            netmask = int("1"*prefix_len*8 + "0"*g_raddr_len*8, 2)
            prefix = did & netmask
            self._gateway_dids[did] = (g_ip, g_port, prefix, prefix_len, g_raddr, g_raddr_len)
            gevent.spawn(self._update_gateways, did, g_ip, g_port, prefix_len)
            return did

    def get_all_gateways(self):
        ret = []
        for did, (ip, port, prefix, prefix_len, raddr, raddr_len) in self._gateway_dids:
            ret.append((int(did), ip, port))
        return ret

    def get_gateway(self, did):
        if did not in self._device_dids:
            print "[DID] cannot find device based on DID %d" % did 
            return None, None, None        
        
        gateway_did, raddr = self._device_dids[did]
        if gateway_did not in self._gateway_dids:
            print "[DID] cannot find gateway based on DID %d" % did 
            return None, None, None

        ip, port, prefix, prefix_len, raddr, raddr_len = self._gateway_dids[gateway_did]
        return gateway_did, ip, port

    def get_device_raddr(self, did):
        if did in self._device_dids:
            gateway_did, raddr = self._device_dids[did]
            return raddr

        print "[DID] cannot find device's radio address based on DID %d" % did 
        return None

    def get_gateway_raddr_len(self, did):
        if did in self._gateway_dids:
            ip, port, prefix, prefix_len, raddr, raddr_len = self._gateway_dids[did]
            return raddr_len

        print "[DID] cannot find gateway's radio address len based on DID %d" % did 
        return None

    def get_gateway_prefix(self, did):
        if did in self._gateway_dids:
            ip, port, prefix, prefix_len, raddr, raddr_len = self._gateway_dids[did]
            return prefix

        print "[DID] cannot find gateway's prefix based on DID %d" % did 
        return None

    def get_gateway_prefix_len(self, did):
        if did in self._gateway_dids:
            ip, port, prefix, prefix_len, raddr, raddr_len = self._gateway_dids[did]
            return prefix_len

        print "[DID] cannot find gateway's prefix len based on DID %d" % did 
        return None

    def create_fwd_message_byte_list(self, did, message):
        header = mptnUtils.create_mult_proto_header_to_str(did, getDIDService().get_master_did(), mptnUtils.MULT_PROTO_MSG_TYPE_FWD, mptnUtils.MULT_PROTO_MSG_SUBTYPE_FWD)
        header = map(ord, header)
        return header+message

def getMockAgent():
    return MockAgent.init()

def getAgent():
    return BrokerAgent.init()

def getZwaveAgent():
    return RPCAgent.init()

def getDeferredQueue():
    return DeferredQueue.init()

def getDIDService():
    return DIDService.init()