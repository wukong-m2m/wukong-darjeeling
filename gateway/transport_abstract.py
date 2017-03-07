import gevent
from gevent.lock import RLock
import mptnUtils as MPTN

class Transport(object):
    def __init__(self, dev_address, name):
        self._global_lock = RLock()
        self._name = name
        self._dev_addr = dev_address
        self._mode = MPTN.STOP_MODE

    def set_update_addr_db(self, update_addr_db):
        self._update_addr_db = update_addr_db

    def get_name(self):
        return self._name

    def get_address(self):
        return self._node_id

    def get_addr_len(self):
        raise NotImplementedError("Please add MPTN.XXXX_ADDRESS_LEN")

    def get_learning_mode(self):
        return self._mode

    def get_learn_handlers(self):
        return {'a':self.add, 'd':self.delete, 's':self.stop}

    def get_rpc_function_lists(self):
        return (self.send, self.getDeviceType, self.routing, self.discover, self.add, self.delete, self.stop, self.poll)

    def recv(self):
        raise NotImplementedError("Please implement in your transport class")

    def send_raw(self, address, payload):
        raise NotImplementedError("Please implement in your transport class")

    def send(self, address, payload):
        raise NotImplementedError("Please implement in your transport class")

    def getDeviceType(self, address):
        raise NotImplementedError("Please implement in your transport class")

    def routing(self):
        raise NotImplementedError("Please implement in your transport class")

    def discover(self):
        discovered_nodes = self._discover()
        self._update_addr_db(discovered_nodes)
        return discovered_nodes

    def _discover(self):
        raise NotImplementedError("Please implement in your transport class")

    def poll(self):
        raise NotImplementedError("Please implement in your transport class")

    def add(self):
        raise NotImplementedError("Please implement in your transport class")

    def delete(self):
        raise NotImplementedError("Please implement in your transport class")

    def stop(self):
        raise NotImplementedError("Please implement in your transport class")    