import sys, os, fcntl
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

import tornado.ioloop
import hashlib
import logging
import collections
import gevent
from gevent.event import AsyncResult
from gevent.server import DatagramServer, StreamServer
import gevent.queue
import wusignal
from configuration import *
from globals import *
import pynvc

import socket
from tinyrpc.protocols.jsonrpc import JSONRPCProtocol
from tinyrpc.transports import ClientTransport
from tinyrpc import RPCClient
import dbdict


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
        return lambda deliver, defer: (deliver.command in allowed_replies) and (deliver.payload != None and deliver.payload[0:2]==defer.message.payload[0:2])


    # to be run in a greenlet thread, context switching with handler
    def receive(self, timeout_msec):
        pass

    # to be run in a thread, and others will use ioloop to monitor pipe of this thread
    def handler(self):
        pass

'''
RPCClientAgent connects to gateway by TCPClient
'''
class RPCAgent(TransportAgent):
    _rpc_agent = None
    @classmethod
    def init(cls):
        if not cls._rpc_agent:
            cls._rpc_agent = RPCAgent()
        return cls._rpc_agent

    def __init__(self):
        self._seq = 0
        self._gw_addr_table = {}
        #self.addGatewayClients(["127.0.0.1:9000"])

        ## TODO
        def receive(self, data, address):
            if src and reply:
                print "[transport] receive: Got message ", src, reply
                # with seq number
                deliver = new_deliver(src, reply[0], reply[1:])
                messages.put_nowait(deliver)
                print '[transport] receive: put a message to messages'

            while True:
                try:
                    src, reply = self.server.receive(timeout_msec)
                except:
                    print "[transport] RPC receive fail, gateway ip=" + client.transport.server_ip + ", port =" + str(client.transport.server_port)

        gevent.spawn(self.handler)


    class TCPClient(ClientTransport):
        def __init__(self, server_ip, server_port):
            self.server_ip = server_ip
            self.server_port = server_port

        def send_message(self, message, expect_reply=True):
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            try:
                # Connect to server and send data
                sock.connect((self.server_ip, self.server_port))
                sock.sendall( + message)
                if expect_reply:
                    # Receive data from the server and shut down
                    received = ""
                    while True:
                        data = sock.recv(4096)
                        if not data: break
                        received += data
            except socket.error, e:
                print "[transport] RPC connection error: " + e
            finally:
                sock.close()

            if expect_reply:
                return received

    def addGatewayClients(self, addresses_list):
        for address in addresses_list:
            addr, port = address.split(":")
            rpc_client = RPCClient(
                JSONRPCProtocol(),
                RPCAgent.TCPClient((addr, int(port)))
            )
            self._gw_addr_table[address] = rpc_client.get_proxy()

    def getGatewayClient(self, address):
        return self._gw_addr_table[address]

    def getAllGatewayClients(self):
        ret = []
        for address, rpc_tuple in self._gw_addr_table.items():
            ret.append((address, rpc_tuple))
        return ret

    def getGatewayAddress(self, node_gid):
        return self._gw_addr_table[getGIDService().findGatewayAddress(node_gid)]


    ### call a remote method
    # result = self.getGatewayClient(gateway's address).method(arg)

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

    def add(self):
        if self._mode != 'stop': return False

        getGIDServce().start_accepting()
        count = 0
        for gw_addr, gwc in self.getAllGatewayClients():
            try:
                if gwc.add():
                    count += 1
            except:
                print "[transport] RPC add() to gateway(%s) fails" % gwc_addr
        
        if count:
            self._mode = 'add'
            return True
        else:
            getGIDServce().stop_accepting()
            return False

    def delete(self):
        if self._mode != 'stop': return False

        getGIDServce().start_accepting()
        count = 0
        for gw_addr, gwc in self.getAllGatewayClients():
            try:
                if gwc.delete():
                    count += 1
            except:
                print "[transport] RPC delete() to gateway(%s) fails" % gwc_addr
        
        if count:
            self._mode = 'delete'
            return True
        else:
            getGIDServce().stop_accepting()
            return False

    def stop(self):
        if self._mode == 'stop': return True

        getGIDServce().stop_accepting()
        count = 0
        for gw_addr, gwc in self.getAllGatewayClients():
            try:
                if gwc.stop():
                    count += 1
            except:
                print "[transport] RPC stop() to gateway(%s) fails" % gwc_addr
                
        if count:
            self._mode = 'stop'
            return True
        else:
            return False

    def poll(self):
        ret = []
        for gw_addr, gwc in self.getAllGatewayClients():
            try:
                p = gwc.poll()
                ret.append(p)
            except:
                print "[transport] RPC poll() to gateway(%s) fails" % gwc_addr
        
        if len(ret):
            return ret.join("\n")
        else:
            return "Not availble"

    def handler(self):
        while True:
            defer = tasks.get()
            #print 'handler: getting defer from task queue'

            if defer.message.command == "discovery":
                #print 'handler: processing discovery request'
                discovered_nodes = []
                for gw_addr, gwc in self.getAllGatewayClients():
                    try:
                        dinterfaces = gwc.discover()
                        for interface, dnodes in dinterfaces.items():
                            for node_lid in dnoes:
                                node_gid = getGIDServce().getGID(node_lid, interface, gw_addr)
                                discovered_nodes.append(node_gid)
                    except:
                        print "[transport] RPC discover() to gateway(%s) fails" % gwc_addr

                defer.callback(discovered_nodes)

            elif defer.message.command == "routing":
                #print 'handler: processing routing request'
                routing = {}
                for gw_addr, gwc in self.getAllGatewayClients():
                    try:
                        rinterfaces = gwc.routing()
                        for interface, rnodes in rinterfaces.items():
                            for node_lid, rinfo in rnodes.items():
                                node_gid = getGIDServce().getGID(node_lid, interface, gw_addr)
                                routing[node_gid] = rinfo
                    except:
                        print "[transport] RPC routing() to gateway(%s) fails" % gw_addr

                defer.callback(routing)

            elif defer.message.command == "device_type":
                node_gid = defer.message.destination
                gw_dest = getGIDService().getLID(node_gid)
                gw_addr = self.getGatewayAddress(node_gid)
                try:
                    device_type = self.getGatewayClient(gw_addr).getDeviceType(gw_dest)
                except:
                    print "[transport] RPC getDeviceType() with node(%d) and gateway(%s) fails" % (node_gid, gw_addr)
                    device_type = None

                defer.callback(device_type)

            else: # defer.message.command is 'send'
                retries = 1
                node_gid = defer.message.destination
                gw_dest = getGIDService().getLID(node_gid)
                command = defer.message.command
                payload = defer.message.payload

                # prevent pyzwave send got preempted and defer is not in queue
                if len(defer.allowed_replies) > 0:
                    print "[transport] handler: appending defer %s to queue" % str(defer)
                    getAgent().append(defer)

                while retries > 0:
                    try:
                        gw_addr = self.getGatewayAddress(node_gid)
                        #print "handler: sending message from defer"
                        success, msg = self.getGatewayClient(gw_addr).send(gw_dest, [command] + payload)

                        if success: break

                        print "[transport] RPC send() replies IOError(%s) so retries %d time(s)" % (str(msg), retries)
                    except:
                        print "[transport] RPC send() to node(%d) and gateway(%s) fails" % (node_gid, gw_addr)
                    retries -= 1

                if retries == 0 or len(defer.allowed_replies) == 0:
                    print "[transport] handler: returns immediately to handle failues, or defer has no expected replies"
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
        return "Not availble"

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
                    elif getGIDService().isGIDdeliver(deliver):
                        print "[transport] GID configuration message received"
                        getGIDService().handleGIDPacket(deliver)
                    else:
                        print "[transport] Failure: Unknown message received"
                else:
                    #log = "Incorrect reply received. Message type correct, but didnt pass verification: " + str(message)
                    print "[transport] message discarded"
                    print '[transport] ' + str(deliver)
            gevent.sleep(0)

class GIDService:
    _gid_server = None
    @classmethod
    def init(cls):
        if not cls._gid_server:
            cls._gid_server = GIDService()
        return cls._gid_server

    def __init__(self):
        self.gid_null = 0x0000
        self.gid_master = 0x0001
        self.gid_bits = 16
        self.gid_max = 2**self.gid_bits - 1
        self.database = dbdict.DBDict("gid.db")
        self.avail_num = 0
        self.avail_bitset = None
        self.loadAvailableGIDs()
        self.reserve_list = []
        self.ready = False
        print 'GIDService: init [OK]'

    def setMode(self, mode):
        if mode == 'add':
            self.ready = True
        else:
            self.ready = False

    def handleGIDPacket(self, packet):
        if not self.ready:
            return

        master_busy()
        payload = None
        client_lid = None
        no_error = True

        if self.isGIDRequestPacket(packet):
            # TO MODIFY LID
            client_lid = packet.destination
            client_gid = self.getGIDSourceAddressFromPacket(packet)
            print "GIDService: got GID REQUEST from %d" % client_lid
            gid = self.reserveGID(client_lid, client_gid)
            if not gid:
                no_error = False
            tmp_msg = [pynvc.MULT_PROTO_MSG_SUBTYPE_GID_OFFER] + self.getTwoBytesListFromInt16(gid)
            payload = self.getGIDPayload(self.getTwoBytesListFromInt16(self.gid_null), self.getTwoBytesListFromInt16(self.gid_master), pynvc.MULT_PROTO_MSG_TYPE, tmp_msg)
            print "GIDService: reply with GID OFFER", payload
        elif self.isGIDACKPacket(packet):
            # TO MODIFY LID
            client_lid = packet.destination
            client_gid = self.getGIDSourceAddressFromPacket(packet)
            print "GIDService: got GID ACK from %d GID = %d" % (client_lid, client_gid)
            self.allocateGID(client_lid, client_gid)
            no_error = False
        else:
            print "GIDService Error: invalid GID packet", packet
            no_error = False

        def cb(reply):
            print "GIDService: GID packet send callback: ", reply
        def error_cb():
            print "GIDService: GID packet send error callback"

        if no_error:
            #getZwaveAgent().deferSend(client_lid, payload[0], payload[1:], [], cb, error_cb)
            defer = new_defer(cb,
                error_cb,
                None,
                [],
                new_message(client_lid, payload[0], payload[1:]), int(round(time.time() * 1000)) + 10000)
            tasks.put_nowait(defer)

        master_available()

    def getTwoBytesListFromInt16(self, integer):
        return [(integer>>8 & 0xFF), (integer & 0xFF)]

    '''
    Bit manipulation functions are used for managing available gids (max 2**16)
    '''
    def getByteAndBitIndexes(self, index):
        byte_ind = int(index / 8)
        bit_ind = index - byte_ind * 8
        return byte_ind, bit_ind

    def testBit(self, index):
        byte_ind, bit_ind = self.getByteAndBitIndexes(index)
        return (ord(self.avail_bitset[byte_ind]) >> bit_ind) & 0x1

    def setBit(self, index):
        byte_ind, bit_ind = self.getByteAndBitIndexes(index)
        self.avail_bitset = self.avail_bitset[0:byte_ind] + chr(ord(self.avail_bitset[byte_ind]) | (0x1 << bit_ind)) + self.avail_bitset[byte_ind+1:]
        self.avail_num += 1

    def clearBit(self, index):
        byte_ind, bit_ind = self.getByteAndBitIndexes(index)
        self.avail_bitset = self.avail_bitset[0:byte_ind] + chr(ord(self.avail_bitset[byte_ind]) & (~(0x1 << bit_ind))) + self.avail_bitset[byte_ind+1:]
        self.avail_num -= 1

    def popSetBit(self):
        for i in range(len(self.avail_bitset)):
            byte = self.avail_bitset[i]
            if byte is not "\x00":
                byte = ord(byte)
                for j in range(8):
                    if byte & 1:
                        ret = i*8 + j
                        self.clearBit(ret)
                        return ret
                    byte >>= 1
        return None

    def loadAvailableGIDs(self):
        gids = map(lambda x: int(x), self.database.keys())
        # exclude GID = 1 (Master) and 0 (NULL)
        #return [i for i in xrange(self.gid_max,1,-1) if i not in gids]
        byte_num = 1 if self.gid_bits <= 3 else (self.gid_bits - 3)
        self.avail_bitset = '\xFF' * (byte_num)
        self.clearBit(0)
        self.clearBit(1)
        self.avail_num = self.gid_max + 1 - 2
        for i in xrange(2,self.gid_max+1):
            if i in gids:
                self.clearBit(i)
    '''
    GID Packet handling sub-functions
    '''
    def reserveGID(self, lid, gid):
        if self.avail_num == 0:
            if len(self.reserve_list) == 0:
                print "GIDService Warning: No Available GID"
                return None
            print "GIDService Warning: Canceling reserved GID"
            for rlid, rgid in self.reserve_list:
                self.setBit(gid)
            self.reserve_list = []

        if (not self.testBit(gid)) and self.database[gid] == lid:
            return gid

        gid = self.popSetBit()
        gevent.sleep(0)
        self.reserve_list.append((lid,gid))
        #print self.reserve_list
        return gid

    def allocateGID(self, lid, gid):
        #print self.reserve_list
        if (lid, gid) not in self.reserve_list:
            print "GIDService Error: client lid = %d, gid = %d not reserved" % (lid,gid)
            return None
        elif gid >= self.gid_max or gid <= 1:
            print "GIDService Error: client gid = %d exceeds range [2, %d]" % (lid,gid,2**self.gid_bits)
            return None
        self.reserve_list.remove((lid, gid))
        self.database[gid] = lid
        print "GIDService: Successfully assign GID %d to LID %d" % (gid, lid)

    def getLID(self, gid):
        # return (interface_name, node_lid)
        if gid in self.database[gid]:
            return self.database[gid]
        else:
            return None

    def findGatewayAddress(self, gid):
        pass

    def getGIDMessageTypeFromPacket(self, packet):
        return packet.payload[4]

    def getGIDSourceAddressFromPacket(self, packet):
        hbyte = packet.payload[1] & 0xFF
        lbyte = packet.payload[2] & 0xFF
        return (((hbyte) << 8) | lbyte)

    def getGIDPayload(self, destination, source, gid_message_type, extra):
        return destination + source + [gid_message_type] + extra

    def isGIDdeliver(self, deliver):
        return deliver.payload[3] == pynvc.MULT_PROTO_MSG_TYPE

    def isGIDRequestPacket(self, packet):
        return self.getGIDMessageTypeFromPacket(packet) == pynvc.MULT_PROTO_MSG_SUBTYPE_GID_REQ

    def isGIDACKPacket(self, packet):
        return self.getGIDMessageTypeFromPacket(packet) == pynvc.MULT_PROTO_MSG_SUBTYPE_GID_ACK

def getMockAgent():
    return MockAgent.init()

def getAgent():
    return BrokerAgent.init()

def getRPCAgent():
    return RPCAgent.init()

def getDeferredQueue():
    return DeferredQueue.init()

def getGIDService():
    return GIDService.init()