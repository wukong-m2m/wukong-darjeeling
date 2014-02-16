from tinyrpc.protocols.jsonrpc import JSONRPCProtocol
from tinyrpc.dispatch import RPCDispatcher
from tinyrpc.exc import RPCError

import cPickle

import gevent
from gevent.queue import Queue
from gevent.event import Event

import gtwconfig as CONST

class RPCService(object):
    def __init__(self, broker):
        self.broker = broker
        self.ingress = Queue()
        self.egress = self.broker.registerService('RPC', self.ingress, [CONST.MULT_PROTO_MSG_SUBTYPE_RPC])
        self._greenlet = None
        self.protocol = JSONRPCProtocol()
        self.dispatcher = RPCDispatcher()

        def send(destination, payload):
            interface_name, node_lid = destination
            try:
                self.broker.getInterface(interface_name).send(node_lid, payload)
            except Exception as e:
                msg = "[RPCService] interface(%s) fails sending to node lid(%d) with error:%s" % (interface, node_lid, str(e))
                print msg
                return (False, msg)
            return (True, None)

        def getDeviceType(destination, payload):
            interface_name, node_lid = destination
            return self.broker.getInterface(interface_name).getDeviceType(node_lid)

        ## for all interfaces
        def routing():
            ri = {}
            for interface_name, interface in self.broker.getAllInterfaces():
                rn = {}
                for node_lid in interface.discover():
                    rn[node_lid] = interface.routing(node_lid)
                ri[interface_name] = rn
                gevent.sleep(0)
            return ri

        def discover():
            d = {}
            for interface_name, interface in self.broker.getAllInterfaces():
                d[interface_name] = interface.discover()
                gevent.sleep(0)
            return d

        def poll():
            p = []
            for interface_name, interface in self.broker.getAllInterfaces():
                msg = interface.poll()
                p.append("%s: %s" % (interface_name, msg))
                gevent.sleep(0)
            return p.join("; ")

        def add():
            return self.broker.nodeInclusion()

        def delete():
            return self.broker.nodeExclusion()

        def stop():
            return self.broker.stopInEx()

        for f in [send, getDeviceType, routing, discover, add, delete, stop, poll]:
            self.dispatcher.add_method(f)

        print "[RPCService] initialized"

    def start(self):
        self._greenlet = gevent.spawn(self.serve_forever)
        print "[RPCService] Started"
        gevent.sleep(0) # Make the greenlet start first and return

    def close(self):
        print "[RPCService] Stop working"
        self._greenlet.kill()

    def serve_forever(self):
        while True:
            address, callback_queue, message = self.ingress.get()
            # It may need to check for security issues
            try:
                request = self.protocol.parse_request(message)
            except RPCError as e:
                response = e.error_respond()
            else:
                response = self.dispatcher.dispatch(request)

            if response != None:
                response = response.serialize()
            else:
                response = str(None)

            assert callback_queue is not None, "[RPCService] cannot return response %s to %s" % (response, str(address))
            callback_queue.put_nowait(response)
            gevent.sleep(0)