try:
  import pyzwave
except:
  print "Please install the pyzwave module in the wukong/tools/python/pyzwave by using"
  print "cd ../tools/python/pyzwave; sudo python setup.py install"
  sys.exit(-1)

import gevent
from gevent import socket
from gevent.server import DatagramServer
from gevent.queue import Queue
import gtwconfig as CONST

class ZWInterface(object):
    def __init__(self, address, name):
        self._name = name
        self._protocol_type = CONST.PROTOCOL_TYPE_ZW
        self._greenlet = None
        self._send_to_broker = None
        self._timeout_msec = 100
        pyzwave.init(address)
        print "[ZWInterface] '%s' initialized on address %s" % (name, address)

    def get_name(self):
        return self._name

    def get_protocol_type(self):
        return self._protocol_type

    def set_send_to_broker(self, f):
        self._send_to_broker = f

    def start(self):
        self._greenlet = gevent.spawn(self._serve_forever)
        print "[ZWInterface] Started"
        gevent.sleep(0) # Make the greenlet start first and return

    def close(self):
        self._greenlet.kill()
        print "[ZWInterface] Stopped"

    def _serve_forever(self):
        while True:
            try:
                src, reply = pyzwave.receive(self._timeout_msec)
                if src and reply:
                    reply = ''.join([chr(byte) for byte in reply])
                    print "[ZWInterface] receive: Got message %s from %d" % (str(reply), src)
                    self._send_to_broker(((self._name, str(src)), reply))
            except:
                print '[ZWInterface] receive exception'

            gevent.sleep(0.01) # sleep for at least 10 msec

    # Function for RPC
    def send(self, destination, payload):
        pyzwave.send(destination, [0x88] + payload)

    def getDeviceType(self, destination):
        return pyzwave.getDeviceType(destination)

    def routing(self, destination):
        routing = pyzwave.routing(destination)
        try:
            routing.remove(gateway_id)
        except ValueError:
            pass
        return routing

    def discover(self):
        nodes = pyzwave.discover()
        gateway_id = nodes[0]
        total_nodes = nodes[1]
        # remaining are the discovered nodes
        discovered_nodes = nodes[2:]
        try:
            discovered_nodes.remove(gateway_id)
        except ValueError:
            pass # sometimes gateway_id is not in the list
        return discovered_nodes

    def poll(self):
        return pyzwave.poll()

    def add_mode(self):
        try:
            pyzwave.add()
            return True
        except:
            return False

    def delete_mode(self):
        try:
            pyzwave.delete()
            return True
        except:
            return False

    def stop_mode(self):
        try:
            pyzwave.stop()
            return True
        except:
            return False