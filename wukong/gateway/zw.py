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

#gevent.server.StreamServer(('localhost', CONST.RPC_PORT), handle)
class ZWInterface(object):
    def __init__(self, address, name, broker):
        self.broker = broker
        self.egress = self.broker.registerInterface(name, self, CONST.RT_IF_TYPE_ZW)
        self._greenlet = None
        self.timeout_msec = 100
        pyzwave.init(address)        
        print "[ZWInterface] initialized on address %s" % address

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
        pyzwave.poll()

    def add(self):
        pyzwave.add()

    def delete(self):
        pyzwave.delete()

    def stop(self):
        pyzwave.stop()

    def start(self):
        self._greenlet = gevent.spawn(self.receive)
        print "[ZWInterface] Started"
        gevent.sleep(0) # Make the greenlet start first and return

    def close(self):
        print "[ZWInterface] Stop working"
        self._greenlet.kill()

    def receive(self):
        while True:
            try:
                src, reply = pyzwave.receive(self.timeout_msec)
                if src and reply:
                    print "[ZWInterface] receive: Got message %s from %d" % (str(reply), src)
                    self.egress.put((src, None, reply))
            except:
                print '[ZWInterface] receive exception'

            gevent.sleep(0.01) # sleep for at least 10 msec