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
import socket # for NetworkServerAgent
import select # for NetworkServerAgent

import pynvc # for message constants
import pyzwave
import pyzigbee

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

class ZwaveAgent(TransportAgent):
    _zwave_agent = None
    @classmethod
    def init(cls):
        if not cls._zwave_agent:
            cls._zwave_agent = ZwaveAgent()
        return cls._zwave_agent

    def __init__(self):
        self._mode = 'stop'

        pyzwave.init(ZWAVE_GATEWAY_IP)

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

    def getDeviceType(self,node):

        result = AsyncResult()

        def callback(reply):
            result.set(reply)

        defer = new_defer(callback,
                callback,
                None,
                None,
                new_message(node, "device_type", 0),
                0)
        tasks.put_nowait(defer)

        return result.get()
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
                    # with seq number
                    deliver = new_deliver(src, reply[0], reply[1:])
                    messages.put_nowait(deliver)
                    print '[transport] receive: put a message to messages'
            except:
                print '[transport] receive exception'

            getDeferredQueue().removeTimeoutDefer()

            #logger.debug('receive: going to sleep')
            gevent.sleep(0.01) # sleep for at least 10 msec


    # to be run in a thread, and others will use ioloop to monitor this thread
    def handler(self):
        while 1:
            defer = tasks.get()
            #print 'handler: getting defer from task queue'

            if defer.message.command == "discovery":
                #print 'handler: processing discovery request'
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
                #print 'handler: processing routing request'
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
            elif defer.message.command == "device_type":
                #print 'handler: processing routing request'
                device_type = pyzwave.getDeviceType(defer.message.destination)
                defer.callback(device_type)
            else:
                #print 'handler: processing send request'
                retries = 1
                destination = defer.message.destination
                command = defer.message.command
                payload = defer.message.payload

                # prevent pyzwave send got preempted and defer is not in queue
                if len(defer.allowed_replies) > 0:
                    print "[transport] handler: appending defer", defer, "to queue"
                    getAgent().append(defer)

                while retries > 0:
                    try:
                        #print "handler: sending message from defer"
                        pyzwave.send(destination, [0x88, command] + payload)

                        break
                    except Exception as e:
                        log = "==IOError== retries remaining: " + str(retries)
                        print '[transport] ' + log
                    retries -= 1

                if retries == 0 or len(defer.allowed_replies) == 0:
                    print "[transport] handler: returns immediately to handle failues, or defer has no expected replies"
                    defer.callback(None)

            gevent.sleep(0)

# Agent to connect to the network server in /wukong/tools/local_network_server
class NetworkServerAgent(TransportAgent):
    MODE_MESSAGE = 1
    MODE_DISCOVERY = 2
    _agent = None
    #TODONR make address and port configurable

    @classmethod
    def init(cls):
        if not cls._agent:
            cls._agent = NetworkServerAgent()
        return cls._agent

    def __init__(self):
        self._mode = 'stop'

        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._socket.connect((NETWORKSERVER_ADDRESS, NETWORKSERVER_PORT))

        # Server says hi
        self._socket.recv(1)

        # Tell the server our network id
        source = 1 # TODONR: is the master ALWAYS 1?
        buffer = bytearray()
        buffer.append(NetworkServerAgent.MODE_MESSAGE)
        buffer.append(source%256)
        buffer.append(source/256)
        self._socket.send(buffer)

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
        return True

    def delete(self):
        if self._mode != 'stop':
            return False
        return True

    def stop(self):
        self._mode = 'stop'
        return True

    def poll(self):
        # TODONR return pyzwave.poll()
        return "Not availble"

    def receive(self, timeout_msec=100):
        while 1:
            try:
                r, w, e = select.select([self._socket], [], [], timeout_msec)
                if r:
                    length = ord(self._socket.recv(1)[0])
                    if length == 0:
                        # just a heartbeat message
                        continue
                    message = map(ord, self._socket.recv(length-1))
                    src = message[0] + (message[1] * 256)
                    reply = message[4:]
                    if src and reply:
                        # with seq number
                        deliver = new_deliver(src, reply[0], reply[1:])
                        messages.put_nowait(deliver)
                        print '[transport] receive: put a message to messages'
            except Exception as e:
                print '[transport] receive exception'
                print e

            getDeferredQueue().removeTimeoutDefer()

            #logger.debug('receive: going to sleep')
            gevent.sleep(0.01) # sleep for at least 10 msec


    # to be run in a thread, and others will use ioloop to monitor this thread
    def handler(self):
        while 1:
            defer = tasks.get()

            if defer.message.command == "discovery":
                discovery_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                discovery_socket.connect((NETWORKSERVER_ADDRESS, NETWORKSERVER_PORT))
                # Server says hi
                discovery_socket.recv(1)

                # Tell the server our network id
                buffer = bytearray()
                buffer.append(NetworkServerAgent.MODE_DISCOVERY)
                discovery_socket.send(buffer)

                discovered_ids=[]
                try:
                    r, w, e = select.select([discovery_socket], [], [], 1000)
                    if r:
                        length = ord(discovery_socket.recv(1)[0])
                        length += 256*ord(discovery_socket.recv(1)[0])
                        message = map(ord, discovery_socket.recv(length-2))
                        for i in range((length-2)/2):
                            discovered_ids.append(message[i*2] + 256*message[i*2+1])
                except Exception as e:
                    print '[transport] receive exception, discovery failed'
                    print e
                finally:
                    discovery_socket.close()
                defer.callback(discovered_ids)
            elif defer.message.command == "routing":
                defer.callback({})
            else: # send
                retries = 1
                destination = defer.message.destination
                command = defer.message.command
                payload = defer.message.payload

                # prevent pyzwave send got preempted and defer is not in queue
                # TODONR: is this relevant for network server?
                if len(defer.allowed_replies) > 0:
                    print "[transport] handler: appending defer", defer, "to queue"
                    getAgent().append(defer)

                while retries > 0:
                    try:
                        destination = defer.message.destination
                        source = 1 # TODONR: is the master ALWAYS 1?
                        buffer = bytearray()
                        buffer.append(1+2+2+1+len(payload))
                        buffer.append(source%256)
                        buffer.append(source/256)
                        buffer.append(destination%256)
                        buffer.append(destination/256)
                        buffer.append(command)
                        buffer.extend(payload)
                        self._socket.send(buffer)
                        break
                    except Exception as e:
                        log = "==IOError== retries remaining: " + str(retries)
                        print '[transport] ' + log
                    retries -= 1

                if retries == 0 or len(defer.allowed_replies) == 0:
                    print "[transport] handler: returns immediately to handle failues, or defer has no expected replies"
                    defer.callback(None)

            gevent.sleep(0)

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
                        print "[transport] what?"
                else:
                    #log = "Incorrect reply received. Message type correct, but didnt pass verification: " + str(message)
                    print "[transport] message discarded"
                    print '[transport] ' + str(deliver)
            gevent.sleep(0)

def getAgent():
    return BrokerAgent.init()

def getMockAgent():
    return MockAgent.init()

def getZwaveAgent():
    return ZwaveAgent.init()

def getNetworkServerAgent():
    return NetworkServerAgent.init()

def getDeferredQueue():
    return DeferredQueue.init()
