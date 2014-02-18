from tinyrpc.protocols.jsonrpc import JSONRPCProtocol
from tinyrpc.dispatch import RPCDispatcher
from tinyrpc.exc import RPCError

import gevent
from gevent.queue import Queue
from gevent.event import Event

import gtwconfig as CONST
import utils

class RPCService(object):
    def __init__(self, name, get_if, get_all_ifs):
        self._name = name
        self._get_if = get_if
        self._get_all_ifs = get_all_ifs
        self._mode = CONST.STOP_MODE
        self._ingress = Queue()
        self._msg_subtypes_of_interest = [CONST.MULT_PROTO_MSG_SUBTYPE_RPC_REQ]
        self._greenlet = None
        self._protocol_handler = JSONRPCProtocol()
        self._dispatcher = RPCDispatcher()

        # for specific interface
        def send(destination, payload):
            interface_name, node_lid = destination
            interface = self._get_if(interface_name)
            if interface is not None:
                status, e = interface.send(node_lid, payload)
                if status:
                    return (True, None)
                msg = "[RPCService] interface(%s) fails sending to node lid(%d) with error: %s" % (interface_name, node_lid, str(e))
                print msg
            else:
                msg = "[RPCService] cannot find interface(%s) to send" % interface_name
            return (False, msg)

        def getDeviceType(destination, payload):
            interface_name, node_lid = destination
            interface = self._get_if(interface_name)
            if interface is not None:
                status, e = interface.getDeviceType(node_lid, payload)
                if status:
                    return (True, None)
                msg = "[RPCService] interface(%s) fails getting device type of node lid(%d) with error: %s" % (interface_name, node_lid, str(e))
                print msg
            else:
                msg = "[RPCService] cannot find interface(%s) to send" % interface_name
            return (False, msg)

        ## for all interfaces
        def routing():
            ri = {}
            for interface_name, interface in self._get_all_ifs():
                rn = {}
                for node_lid in interface.discover():
                    rn[node_lid] = interface.routing(node_lid)
                ri[interface_name] = rn
            return ri

        def discover():
            d = {}
            for interface_name, interface in self._get_all_ifs():
                d[interface_name] = interface.discover()
            return d

        def poll():
            p = []
            for interface_name, interface in self._get_all_ifs():
                msg = interface.poll()
                p.append("%s: %s" % (interface_name, msg))
            return p.join("; ")

        def add():
            success_list = []
            for n, i in self._get_all_ifs():
                if i.add_mode():
                    success_list.append(i)
                else:
                    print "[RPCService] node inclusion error from interface %s" % n

            if len(success_list):
                self._mode = CONST.ADD_MODE
                return True

            return False

        def delete():
            success_list = []
            for n, i in self._get_all_ifs():
                if i.delete_mode():
                    success_list.append(i)
                else:
                    print "[RPCService] node exclusion error from interface %s" % n

            if len(success_list):
                self._mode = CONST.DEL_MODE
                return True

            return False

        def stop():
            success_list = []
            for n, i in self._get_all_ifs():
                if i.stop_mode():
                    success_list.append(i)
                else:
                    print "[RPCService] stop incl./excl. error from interface %s" % n

            if len(success_list) == len(self._interfaces):
                self._mode = CONST.STOP_MODE
                return True

            return False

        for f in [send, getDeviceType, routing, discover, add, delete, stop, poll]:
            self._dispatcher.add_method(f)

        print "[RPCService] initialized"

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
        print "[RPCService] Started"
        gevent.sleep(0) # Make the greenlet start first and return

    def close(self):
        print "[RPCService] Stop working"
        self._greenlet.kill()

    def serve_forever(self):
        while True:
            address, message = self._ingress.get()
            # It may need to check for security issues
            try:
                request = self._protocol_handler.parse_request(message)
            except RPCError as e:
                response = e.error_respond()
            else:
                response = self._dispatcher.dispatch(request)

            if response != None:
                response = response.serialize()
            else:
                response = str(None)

            header = utils.create_mult_proto_header_str(CONST.MASTER_GID, CONST.GATEWAY_GID, CONST.MULT_PROTO_MSG_TYPE, CONST.MULT_PROTO_MSG_SUBTYPE_RPC_REP)

            self._send_to_broker((address, header+response))
            gevent.sleep(0)