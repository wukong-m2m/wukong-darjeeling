# vim: ts=4 sw=4
import sys, os, fcntl
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
import pickle
import tornado.ioloop
import hashlib
import logging
from collections import namedtuple
import gevent
from gevent.event import AsyncResult
from gevent.queue import Queue
import wusignal
import time
from configuration import *
from globals import *
from array import array

import pynvc # for message constants
import pyzwave
import pyzigbee
import dbdict

Message = namedtuple('Message', 'destination command payload')
Defer = namedtuple('Defer', 'callback error_cb verify allowed_replies message timeout')

tasks = Queue()
messages = Queue()


def new_defer(*args):
    return Defer(*args)

def new_message(*args):
    return Message(*args)

def new_deliver(*args):
    return Message(*args)

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
                print 'remove timeouted defer', defer
                # call error cb
                defer.error_cb()
                del self.queue[key]

    def find_defer(self, deliver):
        print 'finding defer for message in queue', self.queue
        for defer_id, defer in self.queue.items():
            if defer.verify(deliver, defer):
                print 'found'
                return defer_id, defer
            else:
                print "Either one of " + str(defer.allowed_replies) + " expected from defer " + str(defer) + " does not match or the sequence number got skewed: " + str(deliver)
        print 'not found'
        return False, False

    def add_defer(self, defer):
        queue_id = str(len(self.queue)) + hashlib.md5(str(defer.message.destination) + str(defer.message.command)).hexdigest()
        print "adding to queue: queue_id ", str(queue_id)
        self.queue[queue_id] = defer
        return queue_id

    def remove_defer(self, defer_id):
        print 'remove_defer'
        if defer_id in self.queue:
            print 'removing defer', self.queue[defer_id]
            del self.queue[defer_id]
            return defer_id
        else:
            return False

    def get_defer(self, defer_id):
        if defer_id in self.queue:
            return self.queue[defer_id]
        else:
            return False

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
        return lambda deliver, defer: (deliver.command in allowed_replies and deliver.command != pynvc.WKPF_ERROR_R) and (deliver.payload != None and deliver.payload[0:2]==defer.message.payload[0:2])


    # to be run in a greenlet thread, context switching with handler
    def receive(self, timeout_msec):
        pass

    # to be run in a thread, and others will use ioloop to monitor pipe of this thread
    def handler(self):
        pass

class ZwaveAgent(TransportAgent):
    _zwave_agent = None
    @classmethod
    def init(cls):
        if not cls._zwave_agent:
            cls._zwave_agent = ZwaveAgent()
        return cls._zwave_agent

    def __init__(self):
        self._mode = 'stop'

        # pyzwave
        try:
            pyzwave.init(ZWAVE_GATEWAY_IP)
        except IOError as e:
            return False

        TransportAgent.__init__(self)
        print 'ZWaveAgent init [OK]'

    # add a defer to queue
    def deferSend(self, destination, command, payload, allowed_replies, cb, error_cb):
        def callback(reply):
            cb(reply)

        def error_callback():
            error_cb()

        defer = new_defer(callback, 
                error_callback,
                self.verify(allowed_replies), 
                allowed_replies, 
                new_message(destination, command, self.getNextSequenceNumberAsPrefixPayload() + payload), int(round(time.time() * 1000)) + 10000)
        tasks.put_nowait(defer)
        return defer

    def send(self, destination, command, payload, allowed_replies):
        result = AsyncResult()

        def callback(reply):
            result.set(reply)

        def error_callback():
            result.set(None)


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

        result = AsyncResult()

        def callback(reply):
            result.set(reply)

        defer = new_defer(callback,
                callback,
                None, 
                None, 
                new_message(1, "discovery", 0),
                0)
        tasks.put_nowait(defer)

        nodes = result.get() # blocking
        return nodes

    def add(self):
        if self._mode != 'stop':
            return False

        try:
            pyzwave.add()
            self._mode = 'add'
            return True
        except:
            return False

    def delete(self):
        if self._mode != 'stop':
            return False

        try:
            pyzwave.delete()
            self._mode = 'delete'
            return True
        except:
            return False

    def stop(self):
        try:
            pyzwave.stop()
            self._mode = 'stop'
            return True
        except:
            return False

    def poll(self):
        try:
            return pyzwave.poll()
        except:
            return "Not availble"

    def receive(self, timeout_msec=100):
        while 1:
            try:
                src, reply = pyzwave.receive(timeout_msec)
                if src and reply:
                    print "receive: Got message", src, reply
                    # with seq number
                    deliver = new_deliver(src, reply[0], reply[1:])
                    messages.put_nowait(deliver)
                    print 'receive: put a message to messages'
            except:
                print 'receive exception'

            getDeferredQueue().removeTimeoutDefer()

            #logger.debug('receive: going to sleep')
            gevent.sleep(0.01) # sleep for at least 10 msec


    # to be run in a thread, and others will use ioloop to monitor this thread
    def handler(self):
        while 1:
            defer = tasks.get()
            print 'handler: getting defer from task queue'

            if defer.message.command == "discovery":
                print 'handler: processing discovery request'
                nodes = pyzwave.discover()
                gateway_id = nodes[0]
                total_nodes = nodes[1]
                # remaining are the discovered nodes
                discovered_nodes = nodes[2:]
                try:
                    discovered_nodes.remove(gateway_id)
                except ValueError:
                    pass # sometimes gateway_id is not in the list
                defer.callback(discovered_nodes)
            elif defer.message.command == "routing":
                print 'handler: processing routing request'
                routing = {}
                nodes = pyzwave.discover()
                gateway_id = nodes[0]
                nodes = nodes[2:]
                try:
                    nodes.remove(gateway_id)
                except ValueError:
                    pass # sometimes gateway_id is not in the list
                for node in nodes:
                    routing[node] = pyzwave.routing(node)
                    try:
                        routing[node].remove(gateway_id)
                    except ValueError:
                        pass
                defer.callback(routing)
            else:
                print 'handler: processing send request'
                retries = 1
                destination = defer.message.destination
                command = defer.message.command
                payload = defer.message.payload

                # prevent pyzwave send got preempted and defer is not in queue
                if len(defer.allowed_replies) > 0:
                    print "handler: appending defer", defer, "to queue"
                    getAgent().append(defer)

                while retries > 0:
                    try:
                        print "handler: sending message from defer"
                        pyzwave.send(destination, [0x88, command] + payload)

                        break
                    except Exception as e:
                        log = "==IOError== retries remaining: " + str(retries)
                        print log
                    retries -= 1

                if retries == 0 or len(defer.allowed_replies) == 0:
                    print "handler: returns immediately to handle failues, or defer has no expected replies"
                    defer.callback(None)

            gevent.sleep(0)

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
        print 'BrokerAgent init [OK]'

    def append(self, defer):
        getDeferredQueue().add_defer(defer)

    def run(self):
        while 1:
            # monitor pipes from receive
            deliver = messages.get()
            print 'getting messages from nodes'
            print str(deliver)

            # display logs from nodes if received
            if deliver.command == pynvc.LOGGING:
                print '[logger] node %d : %s' % (deliver.destination,
                            str(bytearray(deliver.payload)))

            # find out which defer it is for
            defer_id, defer = getDeferredQueue().find_defer(deliver)

            if defer_id and defer:
                # call callback
                defer.callback(deliver)

                # remove it
                getDeferredQueue().remove_defer(defer_id)
            else:
                # if it is special messages
                if not is_master_busy():
                    if deliver.command == pynvc.GROUP_NOTIFY_NODE_FAILURE:
                        print "reconfiguration message received"
                        wusignal.signal_reconfig()
                    elif GIDService.init().isGIDdeliver(deliver):
                        print "GID configuration message received"
                        GIDService.init().handleGIDPacket(deliver)
                    else:
                        print "what?"
                else:
                    #log = "Incorrect reply received. Message type correct, but didnt pass verification: " + str(message)
                    print "message discarded"
                    print str(deliver)
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
        print 'GIDService: init [OK]'

    def handleGIDPacket(self, packet):
        master_busy()
        payload = None
        client_lid = None
        no_error = True

        if self.isGIDRequestPacket(packet):
            client_lid = packet.destination
            client_gid = self.getGIDSourceAddressFromPacket(packet)
            print "GIDService: got GID REQUEST from %d" % client_lid
            gid = self.reserveGID(client_lid, client_gid)
            if not gid: 
                no_error = False
            tmp_msg = [pynvc.GID_OFFER] + self.getTwoBytesListFromInt16(gid)
            payload = self.getGIDPayload(self.getTwoBytesListFromInt16(self.gid_null), self.getTwoBytesListFromInt16(self.gid_master), pynvc.GID_MSG, tmp_msg)
            print "GIDService: reply with GID OFFER", payload
        elif self.isGIDACKPacket(packet):
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

    def getGIDMessageTypeFromPacket(self, packet):
        return packet.payload[4]

    def getGIDSourceAddressFromPacket(self, packet):
        hbyte = packet.payload[1] & 0xFF
        lbyte = packet.payload[2] & 0xFF
        return (((hbyte) << 8) | lbyte)

    def getGIDPayload(self, destination, source, gid_message_type, extra):
        return destination + source + [gid_message_type] + extra

    def isGIDdeliver(self, deliver):
        return deliver.payload[3] == pynvc.GID_MSG

    def isGIDRequestPacket(self, packet):
        return self.getGIDMessageTypeFromPacket(packet) == pynvc.GID_REQUEST

    def isGIDACKPacket(self, packet):
        return self.getGIDMessageTypeFromPacket(packet) == pynvc.GID_ACK

def getAgent():
    return BrokerAgent.init()

def getZwaveAgent():
    return ZwaveAgent.init()

def getDeferredQueue():
    return DeferredQueue.init()