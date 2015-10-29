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
import mptnUtils as MPTN
from transport_abstract import Transport
import traceback
import color_logging, logging
logger = logging

import pprint

ZWAVE_RECV_TIMEOUT = 100

class ZWTransport(Transport):
    def __init__(self, dev_address, name):
        super(ZWTransport, self).__init__(dev_address, name)
        try:
            pyzwave.setVerbose(0)
            pyzwave.setdebug(0)
        except:
            print "PyZwave module has been updated. Please RE-INSTALL the pyzwave module in the wukong/tools/python/pyzwave"
            print "Using command: sudo python setup.py install"
            exit(-1)

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

        logger.info("transport interface %s initialized on %s with Network ID %s and Node ID %s" % (name, dev_address, hex(self._network_id), hex(self._node_id)))

    def get_addr_len(self):
        return MPTN.ZW_ADDRESS_LEN

    def recv(self):
        with self._global_lock:
            try:
                src, reply = pyzwave.receive(ZWAVE_RECV_TIMEOUT)

                if src and reply:
                    logger.debug("receives message %s from address %X" % (reply, src))
                    reply = "".join(map(chr, reply))
                    return (src, reply)
            except Exception as e:
                ret = traceback.format_exc()
                logger.error("receives exception %s\n%s" % (str(e), ret))
        return (None, None)

    def send_raw(self, address, payload):
        ret = None
        with self._global_lock:
            try:
                logger.info("sending %d bytes %s to %X" % (len(payload), payload, address))
                pyzwave.send(address, payload)
            except Exception as e:
                ret = traceback.format_exc()
                logger.error("send_raw exception %s\n%s" % (str(e), ret))
        return ret

    def send(self, address, payload):
        ret = self.send_raw(address, [0x88]+payload)
        if ret is None: return (True, None)

        msg = "%s fails to send to address %d with error %s\n\tmsg: %s" % (self._name, address, ret, payload)
        logger.error(msg)
        return (False, msg)

    def getDeviceType(self, address):
        ret = None
        with self._global_lock:
            try:
                ret = pyzwave.getDeviceType(address)
            except Exception as e:
                logger.error("getDeviceType exception %s\n%s" % (str(e), traceback.format_exc()))
        return ret

    def getNodeRoutingInfo(self, address):
        ret = []
        with self._global_lock:
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

    def _discover(self):
        ret = []

        if not self.stop():
            logger.error("cannot discover without STOP mode")
            return ret

        with self._global_lock:
            nodes = pyzwave.discover()
            zwave_controller = nodes[0]
            total_nodes = nodes[1]
            # remaining are the discovered nodes
            ret = nodes[2:]
            logger.debug("zwave_controller: %s, total_nodes: %s, ret: %s" % (str(zwave_controller), str(total_nodes), str(ret)))
            try:
                ret.remove(zwave_controller)
            except ValueError:
                pass # sometimes zwave_controller is not in the list
        return ret

    def poll(self):
        ret = None
        with self._global_lock:
            try:
                ret = pyzwave.poll()
                ret.replace("\n", " ")
            except:
                pass
        return ret

    def add(self):
        ret = False
        with self._global_lock:
            try:
                pyzwave.add()
                self._mode = MPTN.ADD_MODE
                ret = True
            except Exception as e:
                logger.error("fails to be ADD mode, now in %s mode error: %s\n%s" % (self._mode[1],
                    str(e), traceback.format_exc()))
        return ret

    def delete(self):
        ret = False
        with self._global_lock:
            try:
                pyzwave.delete()
                self._mode = MPTN.DEL_MODE
                ret = True
            except Exception as e:
                logger.error("fails to be DEL mode, now in %s mode error: %s\n%s" % (self._mode[1],
                    str(e), traceback.format_exc()))
        return ret

    def stop(self):
        ret = False
        with self._global_lock:
            try:
                pyzwave.stop()
                self._mode = MPTN.STOP_MODE
                ret = True
            except Exception as e:
                logger.error("fails to be STOP mode, now in %s mode error: %s\n%s" % (self._mode[1],
                    str(e), traceback.format_exc()))
        return ret
