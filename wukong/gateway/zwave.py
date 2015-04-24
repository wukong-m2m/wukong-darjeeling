try:
    import pyzwave
except:
    print "Please install the pyzwave module in the wukong/tools/python/pyzwave by using"
    print "cd ../tools/python/pyzwave; sudo python setup.py install"
    exit(-1)

import gevent
from gevent.lock import RLock
import sys
import pprint
import gtwconfig as CONFIG
import mptnUtils as MPTN

import logging
logging.basicConfig(level=CONFIG.LOG_LEVEL)
logger = logging.getLogger( __name__ )

TIMEOUT = 100

_global_lock = None

class ZWTransport(object):
    def __init__(self, dev_address, name):
        self._name = name
        self._mode = MPTN.STOP_MODE
        global _global_lock
        _global_lock = RLock()
        pyzwave.setVerbose(0)
        pyzwave.setdebug(0)
        pyzwave.init(dev_address)

        try:
            _addr = pyzwave.getAddr()
        except:
            print "PyZwave module has been updated. Please RE-INSTALL the pyzwave module in the wukong/tools/python/pyzwave"
            print "Using command: sudo python setup.py install"
            exit(-1)

        b = _addr[:4]
        self._network_id = sum(b[i] << ((len(b)-1-i) * 8) for i in range(len(b)))
        self._node_id = _addr[4]

        logger.info("Zwave transport interface %s initialized on %s with Network ID %s and Node ID %s" % (name, dev_address, hex(self._network_id), hex(self._node_id)))

    def get_name(self):
        return self._name

    def get_address(self):
        return self._node_id

    def get_addr_len(self):
        return MPTN.RADIO_ADDRESS_LEN_ZW

    def get_learning_mode(self):
        return self._mode

    def recv(self):
        with _global_lock:
            try:
                src, reply = pyzwave.receive(TIMEOUT)

                if src and reply:
                    logger.debug("receives message %s from address %X" % (reply, src))
                    reply = ''.join([chr(byte) for byte in reply])
                    return (src, reply)
            except:
                ret = sys.exc_info()[0]
                logger.error("receives exception %s", str(e))
        return (None, None)

    def send_raw(self, address, payload):
        ret = None
        with _global_lock:
            try:
                logger.info("sending %d bytes %s to %X" % (len(payload), payload, address))
                pyzwave.send(address, payload)
            except:
                ret = sys.exc_info()[0]
        return ret

    def send(self, address, payload):
        ret = self.send_raw(address, [0x88]+payload)
        if ret is None: return (True, None)

        msg = "%s fails to send to address %d with error %s\n\tmsg: %s" % (self._transport.get_name(), address, ret, payload)
        logger.error(msg)
        return (False, msg)

    def getDeviceType(self, address):
        ret = None
        with _global_lock:
            try:
                ret = pyzwave.getDeviceType(address)
            except:
                pass
        return ret

    def getNodeRoutingInfo(self, address):
        ret = []
        with _global_lock:
            ret = pyzwave.routing(address)
            try:
                ret.remove(gateway_id)
            except ValueError:
                pass
        return ret

    def routing(self):
        ret = {}
        for node_raddr in self.discover():
            ret[node_raddr] = self.getNodeRoutingInfo(node_raddr)
        return ret

    def discover(self):
        ret = []
        with _global_lock:
            nodes = pyzwave.discover()
            zwave_controller = nodes[0]
            total_nodes = nodes[1]
            # remaining are the discovered nodes
            ret = nodes[2:]
            logger.debug("---------------------" + str(zwave_controller), str(total_nodes), str(ret))
            try:
                ret.remove(zwave_controller)
            except ValueError:
                pass # sometimes zwave_controller is not in the list
        return ret

    def poll(self):
        ret = None
        with _global_lock:
            try:
                ret = pyzwave.poll()
            except:
                pass
        return ret

    def add_mode(self):
        ret = False
        with _global_lock:
            try:
                pyzwave.add()
                self._mode = MPTN.ADD_MODE
                ret = True
            except:
                logger.error("fails to be ADD mode, now in %s mode", self._mode[1])
        return ret

    def delete_mode(self):
        ret = False
        with _global_lock:
            try:
                pyzwave.delete()
                self._mode = MPTN.DEL_MODE
                ret = True
            except:
                logger.error("fails to be DEL mode, now in %s mode", self._mode[1])
        return ret

    def stop_mode(self):
        ret = False
        with _global_lock:
            try:
                pyzwave.stop()
                self._mode = MPTN.STOP_MODE
                ret = True
            except:
                logger.error("fails to be STOP mode, now in %s mode", self._mode[1])
        return ret

    def get_learn_handlers(self):
        return {'a':self.add_mode, 'd':self.delete_mode, 's':self.stop_mode}

    def get_rpc_function_lists(self):
        return (self.send, self.getDeviceType, self.routing, self.discover, self.add, self.delete, self.stop, self.poll)