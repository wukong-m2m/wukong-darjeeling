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
    def __init__(self,nodeid,ip,port):
        self.nodeid = nodeid
        self.ip = ip
        self.port = port
        self.wuclasses=[]
        self.wuobjects=[]

class UDPTransport(object):
    def __init__(self, dev_address, name):
        self._name = name
        self._dev_addr = dev_address
        self._mode = MPTN.STOP_MODE
        global _global_lock
        _global_lock = RLock()

        try:
            nc_if = netifaces.ifaddresses(dev_address)[netifaces.AF_INET][0]
            self._node_id = (nc_if['addr'], nc_if['netmask'])
        except (IndexError, KeyError, ValueError):
            logger.error("cannot find any IP address from the IP network interface %s" % CONFIG.TRANSPORT_INTERFACE_ADDR)
            self._clear_settings_db()
            exit(-1)
        self._ip = self._node_id[0]
        self._port = MPTN.MPTN_UDP_PORT
        self.enterLearnMode = False
        self.last_node_id = 0
        self.devices=[]
        self.loadDevice()
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
        try:
            data, addr = self.sock.recvfrom(1024)

            t = ord(data[9])
            print "data=",data
            print "t=",t
            if data != "":
                logger.debug("receives message %s from address %s" % (data, str(addr)))
                if t == 1:
                    data = data[10:]
                    print "MPTN message: ", ord(data[0]), ord(data[1])
                    return (MPTN.ID_FROM_STRING(addr[0]), data)
                elif t == 2:
                    self.refreshDeviceData(data)
                    return (None,None)
                else:
                    return (None,None)
        except Exception as e:
            ret = traceback.format_exc()
            logger.error("receives exception %s\n%s" % (str(e), ret))
            self.sock.close()
            self._init_socket()

        return (None, None)

    def send_raw(self, address, payload):
        ret = None
        with _global_lock:
            try:
                message = "".join(map(chr, payload))
                logger.info("sending %d bytes %s to %s" % (len(message), message, MPTN.ID_TO_STRING(address)))
                sock = socket.socket(socket.AF_INET, # Internet
                                    socket.SOCK_DGRAM) # UDP
                sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                sock.sendto(message, (MPTN.ID_TO_STRING(address), MPTN.MPTN_UDP_PORT))
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

    def getDeviceType(self, address):
        ret = None
        logger.info('get device type for %x' % address)
        with _global_lock:
            try:
                # ret = pyzwave.getDeviceType(address)
                ret = 0xff
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
        if ord(data[0]) != 0xAA or ord(data[1]) != 0x55:
            # Drop the unknown packet
            print "Get unknown packet ",data[0],data[1]
            return
        data = data[2:]
        nodeid = ord(data[0])
        ip = struct.unpack("I",data[1:5])
        port = struct.unpack('H',data[5:7])
        type = ord(data[7])
        print "type=",type

        # refresh the values in the IP table
        found = False
        for d in self.devices:
            if d.nodeid == nodeid:
                d.ip = ip
                d.port = port
                found = True
                self.last_node_id = d.nodeid
                self._mode = MPTN.STOP_MODE
                return

        if type == 2 and not found and self.enterLearnMode and self._mode == MPTN.ADD_MODE:
            newid = 2
            while True:
                found = False
                for d in self.devices:
                    if d.nodeid == newid:
                        newid = newid + 1
                        found = True
                        break
                if not found:
                    newd = UDPDevice(newid,ip,port)
                    self.devices.append(newd)
                    self.saveDevice()
                    break
                pass
            self.last_node_id = newid
            self._mode = MPTN.STOP_MODE
        elif type == 2 and found and self.enterLearnMode and self._mode == MPTN.DEL_MODE:
            for i in range(len(self.devices)):
                d = self.devices[i]
                if d.nodeid == nodeid:
                    del self.devices[i]
                    self.saveDevice()
                    return
                pass
            pass
            self.last_node_id = 0
            self._mode = MPTN.STOP_MODE

    def saveDevice(self):
        f = open('devices.pk','w')
        pickle.dump(self.devices,f)
        f.close()

    def loadDevice(self):
        try:
            f = open('devices.pk')
            self.devices = pickle.load(f)
            f.close()
        except:
            pass
    def discover(self):
        # a list = MPTN.get_all_addresses()
        ret = []
        with _global_lock:
            ret=[]
            for i in range(len(self.devices)):
                ret.append(self.devices[i].nodeid)
            # nodes = pyzwave.discover()
            # zwave_controller = nodes[0]
            # total_nodes = nodes[1]
            # # remaining are the discovered nodes
            # ret = nodes[2:]
            # logger.debug("---------------------%s, %s, %s" % (str(zwave_controller), str(total_nodes), str(ret)))
            # try:
            #     ret.remove(zwave_controller)
            # except ValueError:
                pass # sometimes zwave_controller is not in the list
        return ret

    def poll(self):
        ret = None
        print "polled"
        with _global_lock:
            if self._mode == MPTN.STOP_MODE:
                ret = 'Node: %d' % self.last_node_id
            else:
                ret = 'ready'
            pass
        print 'ret=',ret
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
                logger.error("fails to be DEL mode, now in %s mode error: %s\n%s" % (self._mode[1],
                    str(e), traceback.format_exc()))
        return ret

    def get_learn_handlers(self):
        return {'a':self.add, 'd':self.delete, 's':self.stop}

    def get_rpc_function_lists(self):
        return (self.send, self.getDeviceType, self.routing, self.discover, self.add, self.delete, self.stop, self.poll)
