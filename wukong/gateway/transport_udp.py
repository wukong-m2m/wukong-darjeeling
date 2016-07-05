try:
    import netifaces
except:
    print "Please install the netifaces module from pypi"
    print "e.g. sudo pip install netifaces"
    exit(-1)

import gevent
from gevent.lock import RLock
import sys
import gtwconfig as CONFIG
import mptnUtils as MPTN
from transport_abstract import Transport
import traceback
import color_logging, logging
logger = logging

from gevent import socket
import struct
import json

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
        # self.wuclasses=[]
        # self.wuobjects=[]

class UDPTransport(Transport):
    def __init__(self, dev_address, name):
        super(UDPTransport, self).__init__(dev_address, name)
        self.enterLearnMode = False
        self.last_host_id = 0

        self._device_filename = "table_udp_devices.json"
        self.devices=[]
        self._devices_lookup = {}
        if not os.path.isfile(self._device_filename):
            with open(self._device_filename, "w") as f:
                json.dump([], f, sort_keys=True,indent=2)
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

    def get_addr_len(self):
        return MPTN.IP_ADDRESS_LEN

    def recv(self):
        while True:
            try:
                data, addr = self.sock.recvfrom(1024)
                if ord(data[0]) != 0xAA or ord(data[1]) != 0x55:
                    # Drop the unknown packet
                    logger.error("Get unknown packet %s" % (map(ord, data[:2])))
                    continue

                host_id = ord(data[2])
                ip = struct.unpack("<I",data[3:7])[0]
                port = struct.unpack('<H',data[7:9])[0]
                t = ord(data[9])
                logger.debug("recv type=%s, data=%s, addr=%s:%d, short_id=%d, ip=%d, port=%d" % (t, map(ord, data), addr[0], addr[1], host_id, ip, port))
                node_id = self._prefix | host_id
                if data != "":
                    # logger.debug("recv message %s from address %s" % (data, str(addr)))
                    if t == 1:
                        # data[10] is the size of the payload
                        # block unknown sender
                        d_ip, d_port = self.getDeviceAddress(host_id)
                        if d_ip == 0 or d_ip != MPTN.ID_FROM_STRING(addr[0]) or d_port != addr[1]:
                            logger.debug("drop unknown sender's packet")
                            continue
                        data = data[11:]
                        return (node_id, data)

                    elif t == 2:
                        self.refreshDeviceData(host_id, ip, port, t)
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
        with self._global_lock:
            try:
                host_id = address & self._hostmask
                address, port = self.getDeviceAddress(host_id)
                if address == 0: return None

                header = chr(0xaa) + chr(0x55) + chr(host_id) + struct.pack('<I', self._ip) + struct.pack('<H', self._port) + chr(raw_type) + chr(len(payload))
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
        try:
            d = self._devices_lookup.get(host_id)
            return (d.ip, d.port)
        except Exception as e:
            logger.error('Address %d is not registered yet' % host_id)
            return 0,0

    def getDeviceType(self, address):
        ret = None
        logger.info('get device type for %x' % address)
        with self._global_lock:
            try:
                # ret = pyzwave.getDeviceType(address)
                ret = (0xff,0xff,0xff)
                pass
            except Exception as e:
                logger.error("getDeviceType exception %s\n%s" % (str(e), traceback.format_exc()))
        return ret

    def getNodeRoutingInfo(self, address):
        ret = []
        with self._global_lock:
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

    def refreshDeviceData(self,host_id, ip, port, t):

        if host_id == (self._ip & self._hostmask):
            return

        if t != 2:
            return

        found = (host_id in self._devices_lookup)

        if self.enterLearnMode:
            if not found and self._mode == MPTN.ADD_MODE:
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
                            if newid & self._hostmask == 0:
                                logger.error("ID is exhausted")
                                return
                            found = True

                    if not found:
                        newd = UDPDevice(newid,ip,port)
                        self.devices.append(newd)
                        self.saveDevice()
                        self.last_host_id = newid
                        self.stop()
                        logger.debug("device added %s %s %s" % (str(newid),str(MPTN.ID_TO_STRING(ip)),str(port)))
                        break

            elif found and self._mode == MPTN.ADD_MODE:
                for d in self.devices:
                    if d.host_id == host_id:
                        logger.debug("device updated for %s from %s:%s to %s:%s" % (str(host_id),str(MPTN.ID_TO_STRING(d.ip)),str(d.port),str(MPTN.ID_TO_STRING(ip)),str(port)))
                        d.ip = ip
                        d.port = port
                        self.saveDevice()
                        self.last_host_id = d.host_id
                        self.stop()
                        break

            elif found and self._mode == MPTN.DEL_MODE:
                for i in xrange(len(self.devices)):
                    d = self.devices[i]
                    if d.host_id == host_id:
                        self.send_raw(host_id,[0],raw_type=2)
                        del self.devices[i]
                        self.saveDevice()
                        self.last_host_id = host_id
                        self.stop()
                        logger.debug("device deleted %s %s %s" % (str(host_id),str(MPTN.ID_TO_STRING(ip)),str(port)))
                        self.last_host_id = 0
                        return

            self.send_raw(self.last_host_id,[self.last_host_id],raw_type=2)
            return

        elif found: # STOP mode
            for d in self.devices:
                if d.host_id == host_id:
                    if d.ip == ip and d.port == port:
                        logger.debug("device rechecked for %s %s:%s" % (str(host_id),str(MPTN.ID_TO_STRING(d.ip)),str(d.port)))
                        self.last_host_id = 0
                        self.send_raw(d.host_id,[d.host_id],raw_type=2)
                    return

        logger.error("device %s (%s:%s) not allowed to change/add in STOP mode." % (str(host_id),str(MPTN.ID_TO_STRING(ip)),str(port)))
        return

    def saveDevice(self):
        save_devices = [item.__dict__ for item in self.devices]
        with open(self._device_filename,'w') as f:
            json.dump(save_devices, f, sort_keys=True,indent=2)
        self.updateDeviceLookup()

    def loadDevice(self):
        with open(self._device_filename,'r+') as f:
            try:
                load_devices = json.load(f)
            except Exception as e:
                logger.error("loadDevice: %s, %s", str(e), traceback.format_exc())
                load_devices = []
                json.dump(load_devices, f, sort_keys=True,indent=2)
        self.devices = [UDPDevice(item["host_id"],item["ip"],item["port"]) for item in load_devices]
        self.updateDeviceLookup()

    def updateDeviceLookup(self):
        self._devices_lookup.clear()
        for d in self.devices:
            self._devices_lookup[d.host_id] = d

    def _discover(self):
        ret = []

        if not self.stop():
            logger.error("cannot discover without STOP mode")
            return ret

        with self._global_lock:
            for i in xrange(len(self.devices)):
                ret.append(self.devices[i].host_id)

        return ret

    def poll(self):
        ret = None
        with self._global_lock:
            if self._mode != MPTN.STOP_MODE and self.last_host_id == 0:
                ret = 'ready to ' + self._mode[1]
            else:
                ret = "%s" % self._mode[1]
                if self.last_host_id != 0:
                    tmp_node_id = self._prefix | self.last_host_id
                    ret = ret + "\nfound node: %d (ID is %s or %d)" % (self.last_host_id, MPTN.ID_TO_STRING(tmp_node_id), tmp_node_id)

        logger.info("polled. " + ret)
        return ret

    def add(self):
        ret = False
        with self._global_lock:
            try:
                self.enterLearnMode = True
                self._mode = MPTN.ADD_MODE
                self.last_host_id = 0
                ret = True
            except Exception as e:
                logger.error("fails to be ADD mode, now in %s mode error: %s\n%s" % (self._mode[1],
                    str(e), traceback.format_exc()))
        return ret

    def delete(self):
        ret = False
        with self._global_lock:
            try:
                self.enterLearnMode = True
                self._mode = MPTN.DEL_MODE
                self.last_host_id = 0
                ret = True
            except Exception as e:
                logger.error("fails to be DEL mode, now in %s mode error: %s\n%s" % (self._mode[1],
                    str(e), traceback.format_exc()))
        return ret

    def stop(self):
        ret = False
        with self._global_lock:
            try:
                self.enterLearnMode = False
                self._mode = MPTN.STOP_MODE
                ret = True
            except Exception as e:
                logger.error("fails to be STOP mode, now in %s mode error: %s\n%s" % (self._mode[1],
                    str(e), traceback.format_exc()))
        return ret
