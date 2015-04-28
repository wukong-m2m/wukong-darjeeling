import gevent
from gevent.lock import RLock
import sys
import pprint
import gtwconfig as CONFIG
import mptnUtils as MPTN
import socket

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
class UDPTransport(object):
    def __init__(self, dev_address, name):
        self._name = name
        self._mode = MPTN.STOP_MODE
        global _global_lock
        _global_lock = RLock()
        self._node_id=1
        self.devices=[]
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind(('', 3939))
        self.enterLearnMode = False


    def get_name(self):
        return self._name

    def get_radio_address(self):
        return self._node_id

    def get_radio_addr_len(self):
        return MPTN.IP_ADDRESS_LEN

    def get_learning_mode(self):
        return self._mode

    def recv(self):
        _global_lock.acquire(True)
        try:
            #src, reply = pyzwave.receive(TIMEOUT)

            if src and reply:
                logger.debug("UDP receives message %s from radio address %X" % (reply, src))
                reply = ''.join([chr(byte) for byte in reply])
                return (src, reply)
        except:
            e = sys.exc_info()[0]
            logger.error("UDP receives exception %s", str(e))
        finally:
            _global_lock.release()

        return (None, None)

    def send_raw(self, radio_address, payload):
        _global_lock.acquire(True)
        ret = None
        try:
            logger.info("UDP sending %d bytes %s to %X" % (len(payload), payload, radio_address))
            #pyzwave.send(radio_address, payload)
        except:
            e = sys.exc_info()[0]
            ret = "UDP send occurs IO error %s" % e
            logger.error(ret)
        finally:
            _global_lock.release()
        return ret

    def send(self, radio_address, payload):
        self.send_raw(radio_address, [0x88]+payload)

    def get_device_type(self, radio_address):
        _global_lock.acquire(True)
        ret = None
        try:
            #ret = pyzwave.getDeviceType(radio_address)
            pass
        finally:
            _global_lock.release()
        return ret

    def routing(self, radio_address):
        _global_lock.acquire(True)
        routing = []
        try:
            #routing = pyzwave.routing(radio_address)
            try:
                routing.remove(gateway_id)
            except ValueError:
                pass
        finally:
            _global_lock.release()
        return routing
    def refreshDeviceData(self,data):
        if data[0] != 0xAA or data[1] != 0x55:
            # Drop the unknown packet
            print "Get unknown packet ",data
            return
        nodeid = data[2]
        ip = data[3:6]
        port = data[7]*256+data[8]
        # refresh the values in the IP table
        found = False
        for d in self.devices:
            if d.nodeid == nodeid:
                d.ip = ip
                d.port = port
                found = False
                break
            
        type = data[9]
        if type == 2 and not found and self.enterLearnMode and self._mode == MPTN.ADD_MODE:
            obj = {nodeid: newid}
            newid = 2
            while True:
                found = False
                for d in self.devices:
                    if d.nodeid == newid: 
                        newid = newid + 1
                        found = True
                        break
                if not found:
                    newd = {nodeid:newid}
                    newd.ip = ip
                    newd.port = port
                    newd.wuclasses=[]
                    newd.wuobjects=[]
                    break
                pass
            pass
        elif type == 2 and found and self.enterLearnMode and self._mode == MPTN.DEL_MODE:
            for i in range(self.devices):
                d = self.devices[i]
                if d.nodeid == nodeid:
                    del self.devices[i]
                    return
                pass
            pass
        elif type == 1:
            # Handle property change request
            pass

    def discover(self):
        _global_lock.acquire(True)
        discovered_nodes = []
        try:
            #nodes = pyzwave.discover()
            gateway_id = nodes[0]
            total_nodes = nodes[1]
            # remaining are the discovered nodes
            discovered_nodes = nodes[2:]
            print "---------------------", gateway_id, total_nodes, discovered_nodes
            try:
                discovered_nodes.remove(gateway_id)
            except ValueError:
                pass # sometimes gateway_id is not in the list
        finally:
            _global_lock.release()
        return discovered_nodes

    def poll(self):
        _global_lock.acquire(True)
        ret = None
        try:
            #ret = pyzwave.poll()
            pass
        finally:
            _global_lock.release()
        return ret

    def add_mode(self):
        _global_lock.acquire(True)
        ret = False
        try:
            #pyzwave.add()
            self.enterLearnMode = True
            self._mode = MPTN.ADD_MODE
            ret = True
        except:
            logger.error("UDP is not in ADD mode but %s mode", self._mode[1])
        finally:
            _global_lock.release()
        return ret

    def delete_mode(self):
        _global_lock.acquire(True)
        ret = False
        try:
            #pyzwave.delete()
            self.enterLearnMode = True
            self._mode = MPTN.DEL_MODE
            ret = True
        except:
            logger.error("UDP is not in enters DEL mode but %s mode", self._mode[1])
        finally:
            _global_lock.release()
        return ret

    def stop_mode(self):
        _global_lock.acquire(True)
        ret = False
        try:
            #pyzwave.stop()
            self.enterLearnMode = False
            self._mode = MPTN.STOP_MODE
            ret = True
        except:
            logger.error("UDP is not in enters ADD mode but %s mode", self._mode[1])
        finally:
            _global_lock.release()
        return ret
