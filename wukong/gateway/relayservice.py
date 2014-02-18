import gevent
from gevent.queue import Queue
from gevent.event import Event

import gtwconfig as CONST
import utils

class RelayService(object):
    def __init__(self, name):
        self._name = name
        self._ingress = Queue()
        self._msg_subtypes_of_interest = [CONST.MULT_PROTO_MSG_SUBTYPE_RPC_REQ]
        self._greenlet = None
        print "[RelayService] initialized"

    def set_send_to_broker(self, f):
        self._send_to_broker = f

    def get_ingress(self):
        return self._ingress

    def get_name(self):
        return self._name

    def get_msg_subtypes_of_interest(self):
        return self._msg_subtypes_of_interest

    def start(self):
        self._greenlet = gevent.spawn(self.serve_forever)
        print "[RelayService] Started"
        gevent.sleep(0) # Make the greenlet start first and return

    def close(self):
        print "[RelayService] Stop working"
        self._greenlet.kill()

    def serve_forever(self):
        while True:
            address, message = self._ingress.get()



            header = utils.create_mult_proto_header_str(CONST.MASTER_GID, CONST.GATEWAY_GID, CONST.MULT_PROTO_MSG_TYPE, CONST.MULT_PROTO_MSG_SUBTYPE_RPC_REP)

            self._send_to_broker((address, header+response))
            gevent.sleep(0)