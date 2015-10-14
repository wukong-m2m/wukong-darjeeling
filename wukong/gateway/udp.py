import gevent
from gevent.lock import RLock
from gevent import socket
import sys
import gtwconfig as CONFIG
import mptnUtils as MPTN
import struct
import pickle

try:
    import netifaces
except:
    print "Please install the netifaces module from pypi"
    print "e.g. sudo pip install netifaces"
    exit(-1)

import traceback
import color_logging, logging
logger = logging

TIMEOUT = 100

_global_lock = None
# UDP packet format
# BYTE 0-1: Magic number 0xAA 0x55
# BYTE 2: Node ID
#   0: new device
# BYTE 3-6: IP address
# BYTE 7-8: Port
# BYTE 9: payload type
#       1 : Property change
#       2 : Node Info

# Type 1
# BYTE 10: Number of property change
# BYTE 11: Property payload
# BYTE 12: Property number
# BYTE 13: Property Length
# BYTE 14-14+$(11): Property Value
# Type 2:
#    No payload
# The complete design document is available at
#    https://docs.google.com/document/d/1IrsSE-QA0cvoSgMTKLS3NJvTj24pOzujBMnZ8_1AxWk/edit?usp=sharing
class UDPDevice(object):
    def __init__(self,host_id,ip,port):
        self.host_id = host_id
        self.ip = ip
        self.port = port
        self.wuclasses=[]
        self.wuobjects=[]

class UDPTransport(object):
    def __init__(self, dev_address, name):
        global _global_lock
        _global_lock = RLock()

        self._name = name
        self._dev_addr = dev_address

        self._mode = MPTN.STOP_MODE
        self.enterLearnMode = False
        self.last_host_id = 0

        self._device_filename = "devices.pkl"
        self.devices=[]
        self.loadDevice()

        try:
            nc_if = netifaces.ifaddresses(dev_address)[netifaces.AF_INET][0]
            self._node_id = (nc_if['addr'], nc_if['netmask'])
            self._ip = MPTN.ID_FROM_STRING(self._node_id[0])
            self._netmask = MPTN.ID_FROM_STRING(self._node_id[1])

        except (IndexError, KeyError, ValueError):
            logger.error("cannot find any IP address from the IP network interface %s" % CONFIG.TRANSPORT_INTERFACE_ADDR)
            self._clear_settings_db()
            exit(-1)

        self._prefix = self._ip & self._netmask
        self._hostmask = ((1 << (MPTN.IP_ADDRESS_LEN * 8)) - 1) ^ self._prefix
        self._port = MPTN.MPTN_UDP_PORT
        self._init_socket()

    def _init_socket(self):
        self.sock = socket.socket(socket.AF_INET, # Internet
                          socket.SOCK_DGRAM) # UDP
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind(('', self._port))

        logger.info("transport interface %s initialized on %s IP=%s PORT=%d with Node ID %s/%s" % (self._name,
            self._dev_addr, MPTN.ID_TO_STRING(self._ip), self._port,
            MPTN.ID_TO_STRING(self._node_id[0]), str(self._node_id[1])
            )
        )

    def get_name(self):
        return self._name

    def get_address(self):
        return self._node_id

    def get_addr_len(self):
        return MPTN.IP_ADDRESS_LEN

    def get_learning_mode(self):
        return self._mode

    def recv(self):
        while True:
            try:
                data, addr = self.sock.recvfrom(1024)
                if ord(data[0]) != 0xAA or ord(data[1]) != 0x55:
                    # Drop the unknown packet
                    logger.error("Get unknown packet %s" % (map(ord, data[:2])))
                    continue

                host_id = ord(data[2])
                ip = struct.unpack("I",data[3:7])
                port = struct.unpack('H',data[7:9])
                t = ord(data[9])
                logger.debug("recv type=%s, data=%s, ip=%s, port=%d, short_id=%d" % (t, map(ord, data), MPTN.ID_FROM_STRING(addr[0]), addr[1], host_id))
                node_id = self._prefix | host_id
                if data != "":
                    logger.debug("recv message %s from address %s" % (data, str(addr)))
                    if t == 1:
                        # data[10] is the size of the payload
                        # block unknown sender
                        if self.getDeviceAddress(host_id)[0] == 0:
                            logger.debug("drop unknown sender's packet")
                            continue
                        data = data[11:]
                        return (node_id, data)

                    elif t == 2:
                        self.refreshDeviceData(data)
                        continue

                    else:
                        continue

            except Exception as e:
                ret = traceback.format_exc()
                logger.error("receives exception %s\n%s" % (str(e), ret))
                self.sock.close()
                self._init_socket()

        return (None, None)

    def send_raw(self, address, payload, raw_type=1):
        ret = None
        with _global_lock:
            try:
                host_id = address & self._hostmask
                address, port = self.getDeviceAddress(host_id)
                if address == 0: return None

                header = chr(0xaa) + chr(0x55) + chr(host_id) + struct.pack('I', address) + struct.pack('H', port) + chr(raw_type) + chr(len(payload))
                message = "".join(map(chr, payload))
                logger.info("sending %d bytes %s to %s, port %d" % (len(message), map(ord, message), MPTN.ID_TO_STRING(address),port))
                sock = socket.socket(socket.AF_INET, # Internet
                                    socket.SOCK_DGRAM) # UDP
                sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                #sock.sendto(message, (MPTN.ID_TO_STRING(address), MPTN.MPTN_UDP_PORT))
                sock.sendto(header+message, (MPTN.ID_TO_STRING(address), port))
                sock.close()
            except Exception as e:
                ret = traceback.format_exc()
                logger.error("send_raw exception %s\n%s" % (str(e), ret))
        return ret

    def send(self, address, payload):
        ret = self.send_raw(address, payload)
        if ret is None: return (True, None)

        msg = "%s fails to send to address %d with error %s\n\tmsg: %s" % (self._transport.get_name(), address, ret, payload)
        logger.error(msg)
        return (False, msg)

    def getDeviceAddress(self, host_id):
        for d in self.devices:
            if d.host_id == host_id:
                return (d.ip,d.port)
        logger.error('Address %d is not registered yet' % host_id)
        return 0,0
    def getDeviceType(self, address):
        ret = None
        logger.info('get device type for %x' % address)
        with _global_lock:
            try:
                # ret = pyzwave.getDeviceType(address)
                ret = (0xff,0xff,0xff)
                pass
            except Exception as e:
                logger.error("getDeviceType exception %s\n%s" % (str(e), traceback.format_exc()))
        return ret

    def getNodeRoutingInfo(self, address):
        ret = []
        with _global_lock:
            # ret = pyzwave.routing(address)
            # try:
            #     ret.remove(gateway_id)
            # except ValueError:
            pass

        return ret

    def routing(self):
        ret = {}
        for node_raddr in self.discover():
            ret[node_raddr] = self.getNodeRoutingInfo(node_raddr)
        return ret

    def refreshDeviceData(self,data):
        # if ord(data[0]) != 0xAA or ord(data[1]) != 0x55:
        #     # Drop the unknown packet
        #     print "Get unknown packet ",data[0],data[1]
        #     return
        data = data[2:]
        host_id = ord(data[0])
        ip = struct.unpack("I",data[1:5])[0]
        port = struct.unpack('H',data[5:7])[0]
        # t = ord(data[7])
        # print "type=",t
        if host_id == (self._ip & self._hostmask):
            return

        # refresh the values in the IP table
        found = False
        for d in self.devices:
            if d.host_id == host_id:
                d.ip = ip
                d.port = port
                found = True
                self.last_host_id = d.host_id
                self._mode = MPTN.STOP_MODE
                self.send_raw(self.last_host_id,[self.last_host_id],raw_type=2)
                break

        if not found and self.enterLearnMode and self._mode == MPTN.ADD_MODE: # and t == 2
            newid = 1
            while True:
                found = False
                for d in self.devices:
                    if d.host_id == newid:
                        newid = newid + 1
                        if newid & self._hostmask == 0:
                            logger.error("ID is exhausted")
                            return
                        found = True
                        break
                else:
                    if newid == (self._ip & self._hostmask):
                        newid += 1
                        found = True

                if not found:
                    newd = UDPDevice(newid,ip,port)
                    self.devices.append(newd)
                    self.saveDevice()
                    break

            self.last_host_id = newid
            self._mode = MPTN.STOP_MODE

        elif found and self.enterLearnMode and self._mode == MPTN.DEL_MODE: # and t == 2
            for i in range(len(self.devices)):
                d = self.devices[i]
                if d.host_id == host_id:
                    del self.devices[i]
                    self.saveDevice()
                    return
                pass
            pass
            self.last_host_id = 0
            self._mode = MPTN.STOP_MODE

        self.send_raw(self.last_host_id,[self.last_host_id],raw_type=2)

    def saveDevice(self):
        f = open(self._device_filename,'w')
        pickle.dump(self.devices,f)
        f.close()

    def loadDevice(self):
        try:
            f = open(self._device_filename)
            self.devices = pickle.load(f)
            f.close()
        except:
            pass

    def discover(self):
        # a list = MPTN.get_all_addresses()
        ret = []

        if not self.stop():
            logger.error("cannot discover without STOP mode")
            return ret

        with _global_lock:
            for i in range(len(self.devices)):
                ret.append(self.devices[i].host_id)

        return ret

    def poll(self):
        ret = None
        print "polled"
        with _global_lock:
            if self._mode == MPTN.STOP_MODE:
                ret = 'found Node: %d' % self.last_host_id
            else:
                ret = 'ready to ' + self._mode[1]
        logger.info(ret)
        return ret

    def add(self):
        ret = False
        with _global_lock:
            try:
                self.enterLearnMode = True
                self._mode = MPTN.ADD_MODE
                ret = True
            except Exception as e:
                logger.error("fails to be ADD mode, now in %s mode error: %s\n%s" % (self._mode[1],
                    str(e), traceback.format_exc()))
        return ret

    def delete(self):
        ret = False
        with _global_lock:
            try:
                self.enterLearnMode = True
                self._mode = MPTN.DEL_MODE
                ret = True
            except Exception as e:
                logger.error("fails to be DEL mode, now in %s mode error: %s\n%s" % (self._mode[1],
                    str(e), traceback.format_exc()))
        return ret

    def stop(self):
        ret = False
        with _global_lock:
            try:
                self.enterLearnMode = False
                self._mode = MPTN.STOP_MODE
                ret = True
            except Exception as e:
                logger.error("fails to be STOP mode, now in %s mode error: %s\n%s" % (self._mode[1],
                    str(e), traceback.format_exc()))
        return ret

    def get_learn_handlers(self):
        return {'a':self.add, 'd':self.delete, 's':self.stop}

    def get_rpc_function_lists(self):
        return (self.send, self.getDeviceType, self.routing, self.discover, self.add, self.delete, self.stop, self.poll)
