import gevent
from gevent import socket
from gevent.server import StreamServer
from gevent.queue import Queue
import ipaddr
import iputil
import gtwconfig as CONST

#gevent.server.StreamServer(('localhost', CONST.RPC_PORT), handle)
class TCPInterface(object):
    def __init__(self, address, name, broker):
        self.broker = broker
        self.egress = self.broker.registerInterface(name, self, CONST.RT_IF_TYPE_IP)
        self._greenlet = None
        print "[TCPInterface] initialized on address %s:%d" % (address[0], address[1])

    def send(self, destination, payload):
        pass

    def getDeviceType(self, destination, payload):
        pass

    def routing(self, destination):
        pass

    def discover(self):
        pass

    def poll(self):
        return "Not availble"

    def add(self):
        return True

    def delete(self):
        return True

    def stop(self):
        return True


    def start(self):
        self._greenlet = gevent.spawn(self.serve_forever)
        print "[TCPInterface] Started"
        gevent.sleep(0) # Make the greenlet start first and return

    def close(self):
        print "[TCPInterface] Stop working"
        self._greenlet.kill()

    def serve_forever(self):
        while True:
            #address, callback_queue, message = self.ingress.get()

            gevent.sleep(3)
    # def __init__(self, max_buf_size=4096, queue_class=Queue.Queue):
    #     self._queue_class = queue_class
    #     self.rpc_messages = queue_class()
    #     self.max_buf_size = max_buf_size
    
    # def stop(self):
    #     self.rpc_messages.put((None, StopIteration))

    # # RPC functions required to implement ServerTransport
    # def receive_message(self):
    #     return self.rpc_messages.get()

    # def send_reply(self, rpc_context, rpc_reply):
    #     if not isinstance(rpc_reply, str):
    #         raise TypeError('[TCPTransport] RPC str expected')
    #     rpc_context.put(rpc_reply)

    # # TCP transport handler for StreamServer
    # def handle(self, socket, address):        
    #     msg = socket.recv(self.max_buf_size)

    #     if msg[0:3] == "RPC":
    #         msg = msg[3:]
    #         print "[TCPTransport] RPC msg: " + msg
    #         context = self._queue_class()
    #         self.rpc_messages.put((context, msg))
    #         socket.send(context.get())
    #     else:
    #         print "[TCPTransport] Non-RPC msg: " + msg
    #         print msg, address
    #         if msg[0:11] == "MASTERIPSET":
    #             pass

    