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

        # pyzwave
        try:
            pyzwave.init(ZWAVE_GATEWAY_IP)
        except IOError as e:
            return False

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
            else:
                #print 'handler: processing send request'
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
                        #print "handler: sending message from defer"
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
        gevent.spawn(self.run)
        print 'BrokerAgent init'

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
                        print "reconfiguration message received"
                        wusignal.signal_reconfig()
                    else:
                        print "what?"
                else:
                    #log = "Incorrect reply received. Message type correct, but didnt pass verification: " + str(message)
                    print "message discarded"
                    print str(deliver)
            gevent.sleep(0)

def getAgent():
    return BrokerAgent.init()

def getZwaveAgent():
    return ZwaveAgent.init()

def getDeferredQueue():
    return DeferredQueue.init()
