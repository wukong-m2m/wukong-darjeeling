try:
  import pyzwave
except:
  print "Please install the pyzwave module in the wukong/tools/python/pyzwave by using"
  print "cd ../tools/python/pyzwave; sudo python setup.py install"
  exit(-1)

import gevent
from gevent.lock import RLock
import sys

import gtwconfig as CONFIG
import mptn as MPTN

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
        pyzwave.init(dev_address)

        logger.info("Zwave radio interface %s initialized on %s" % (name, dev_address))

    def get_name(self):
        return self._name

    def get_radio_address(self):
        return 0x01

    def get_radio_addr_len(self):
        return MPTN.RADIO_ADDRESS_LEN_ZW

    def get_learning_mode(self):
        return self._mode

    def recv(self):
        _global_lock.acquire(True)
        try:
            src, reply = pyzwave.receive(TIMEOUT)

            if src and reply:
                logger.debug("Zwave receives message %s from radio address %X" % (reply, src))
                reply = ''.join([chr(byte) for byte in reply])
                return (src, reply)
        except:
            e = sys.exc_info()[0]
            logger.error("Zwave receives exception %s", str(e))
        finally:
            _global_lock.release()

        return (None, None)

    def send(self, radio_address, payload):
        _global_lock.acquire(True)
        ret = None
        try:
            pyzwave.send(radio_address, payload)
        except:
            e = sys.exc_info()[0]
            ret = "Zwave send occurs IO error %s" % e
            logger.error(ret)
        finally:
            _global_lock.release()
        return ret

    def get_device_type(self, radio_address):
        _global_lock.acquire(True)
        ret = None
        try:
            ret = pyzwave.getDeviceType(radio_address)
        finally:
            _global_lock.release()
        return ret

    def routing(self, radio_address):
        _global_lock.acquire(True)
        routing = []
        try:
            routing = pyzwave.routing(radio_address)
            try:
                routing.remove(gateway_id)
            except ValueError:
                pass
        finally:
            _global_lock.release()
        return routing

    def discover(self):
        _global_lock.acquire(True)
        discovered_nodes = []
        try:
            nodes = pyzwave.discover()
            gateway_id = nodes[0]
            total_nodes = nodes[1]
            # remaining are the discovered nodes
            discovered_nodes = nodes[2:]
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
            ret = pyzwave.poll()
        finally:
            _global_lock.release()
        return ret

    def add_mode(self):
        _global_lock.acquire(True)
        ret = False
        try:
            pyzwave.add()
            self._mode = MPTN.ADD_MODE
            ret = True
        except:
            logger.error("Zwave is not in ADD mode but %s mode", self._mode[1])
        finally:
            _global_lock.release()
        return ret

    def delete_mode(self):
        _global_lock.acquire(True)
        ret = False
        try:
            pyzwave.delete()
            self._mode = MPTN.DEL_MODE
            ret = True
        except:
            logger.error("Zwave is not in enters DEL mode but %s mode", self._mode[1])
        finally:
            _global_lock.release()
        return ret

    def stop_mode(self):
        _global_lock.acquire(True)
        ret = False
        try:
            pyzwave.stop()
            self._mode = MPTN.STOP_MODE
            ret = True
        except:
            logger.error("Zwave is not in enters ADD mode but %s mode", self._mode[1])
        finally:
            _global_lock.release()
        return ret