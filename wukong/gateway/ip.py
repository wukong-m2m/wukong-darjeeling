import gevent
from gevent import socket
from gevent.server import StreamServer
from gevent.queue import Queue
import iptools
import gtwconfig as CONST

class IPInterface(object):
    def __init__(self, address, name):
        self._name = name
        self._protocol_type = CONST.PROTOCOL_TYPE_IP
        self._greenlet = None
        self._send_to_broker = None
        print "[IPInterface] initialized on address %s:%d" % (address[0], address[1])

    def get_name(self):
        return self._name

    def get_protocol_type(self):
        return self._protocol_type

    def set_send_to_broker(self, f):
        self._send_to_broker = f

    def start(self):
        self._greenlet = gevent.spawn(self._serve_forever)
        print "[IPInterface] Started"
        gevent.sleep(0) # Make the greenlet start first and return

    def close(self):
        print "[IPInterface] Stop working"
        self._greenlet.kill()

    def _serve_forever(self):
        while True:
            #address, callback_queue, message = self.ingress.get()

            gevent.sleep(0)

    # Function for RPC
    def send(self, destination, payload):
        pass

    def getDeviceType(self, destination):
        pass

    def routing(self, destination):
        pass

    def discover(self):
        pass

    def poll(self):
        return "Not availble"

    def add_mode(self):
        return True

    def delete_mode(self):
        return True

    def stop_mode(self):
        return True