import gevent
from gevent import socket
from gevent.server import DatagramServer
from gevent.queue import Queue
import ipaddr
import iputil
import gtwconfig as CONST

#gevent.server.StreamServer(('localhost', CONST.RPC_PORT), handle)
class UDPInterface(object):
    def __init__(self, address, name, broker):
        self.broker = broker
        self.egress = self.broker.registerInterface(name, self, CONST.RT_IF_TYPE_IP)
        self._greenlet = None
        print "[UDPInterface] initialized on address %s:%d" % (address[0], address[1])

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
        print "[UDPInterface] Started"
        gevent.sleep(0) # Make the greenlet start first and return

    def close(self):
        print "[UDPInterface] Stop working"
        self._greenlet.kill()

    def serve_forever(self):
        while True:
            #address, callback_queue, message = self.ingress.get()

            gevent.sleep(3)

class UDPClient():
    def __init__(self, server_ip, server_port):
        self.server_ip = server_ip
        self.server_port = server_port

    def send_message(self, message, expect_reply=True):
        sock = socket.socket(type=socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        try:
            sock.sendto(message, (self.server_ip, self.server_port))
            received, address = sock.recvfrom(4096)
        finally:
            sock.close()

        if expect_reply:
            return received, address

    def connect_master(self):
        pass

class AdServer(DatagramServer):
    def __init__(self, *args, **kwargs):
        self.adsock = socket.socket(type=socket.SOCK_DGRAM)
        self.adsock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.adsock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        self.ifname = CONST.IP_INTERFACE
        self.ip = ipaddr.IPv4Network("%s/%s" % (
                iputil.get_ip_address(self.ifname),
                iputil.get_net_mask(self.ifname)
            )
        )
        self.bcaddr = str(self.ip.broadcast)
        self.bcport = CONST.BROADCAST_IP_PORT

        DatagramServer.__init__(self, *args, **kwargs)

    def customized_start(self):
        gevent.spawn(self.token_forever)
        gevent.spawn(self.serve_forever)

    def token_forever(self):
        while True:
            print "[AdServer] Advertise to %s:%s" % (self.bcaddr, self.bcport)
            self.adsock.sendto('GATEWAY_AD_PORT=%d' % CONST.AD_PORT, (self.bcaddr, self.bcport))
            gevent.sleep(3*60)

    def handle(self, data, address):
        print '[AdServer] from %s: got %r' % (address[0], data)
        #self.socket.sendto('Received %s bytes' % len(data), address)

class UDPServer(DatagramServer):
    def __init__(self, *args, **kwargs):
        DatagramServer.__init__(self, *args, **kwargs)
        self._messages = Queue()
        self._lidtoaddr = {1:"127.0.0.1:9000"}
        self._addrtolid = {"127.0.0.1:9000":1}
        gevent.spawn(self.serve_forever)

    def receive_messages(self):
        return self._messages.get()

    def send_message(self, lid, message):
        address = self.find_address(lid).split(":")
        address[1] = int(address[1])
        address = tuple(address)

        sock = socket.socket(type=socket.SOCK_DGRAM)
        #sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        try:
            sock.sendto(message, address)
            print "[IPAgent] send message %s to %s" % (message, address)
            received, address = sock.recvfrom(4096)
        except Exception as e:
            print e
        finally:
            sock.close()

    def handle(self, data, address):
        print '[IPAgent] from addr %s port %s: got %r' % (address[0], address[1], data)
        straddr = "%d.%d.%d.%d:%d" % tuple([ord(c) for c in data[1:5]]+[(ord(data[5])<<8)+(ord(data[6])&0xff)])
        print straddr
        if ord(data[0]) == IPAgent.LID_REQ:
            lid = self.find_lid(straddr)
            if lid == None: lid = self.allocate_lid(straddr)
            self.socket.sendto(bytearray([IPAgent.LID_REP, (lid>>8)&0xff, lid&0xff]), address)
        else:
            lid = self.find_lid("%s:%s" % (address[0], address[1]))
            self._messages.put((lid, data))

    def find_lid(self, address):
        if address in self._addrtolid:
            return self._addrtolid[address]
        else:
            return None

    def find_address(self, lid):
        if lid in self._lidtoaddr:
            return self._lidtoaddr[lid]
        else:
            return None            

    def allocate_lid(self, address):
        for key in xrange(2, 256):
            if key not in self._lidtoaddr:
                self._lidtoaddr[key] = address
                self._addrtolid[address] = key
                return key
        assert False, "[IPAgent] Not enough lids"

    def discover(self):
        lids = self._lidtoaddr.keys()
        return [1, len(lids)] + lids