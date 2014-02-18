import gevent
from gevent.queue import PriorityQueue, Queue

import gtwconfig as CONST
import utils

class Broker(object):

    def __init__(self):
        self._interfaces = {}
        self._services = {}
        self._start_list = []
        self._ingress = PriorityQueue()
        self._greenlet = None
        self._no_dump = False
        print "[Broker] initialized"

    def register_service(self, service):
        name = service.get_name()
        ingress = service.get_ingress()
        msg_subtypes = service.get_msg_subtypes_of_interest()
        print "[Broker] register service type(s) %s under name '%s'" % (str(msg_subtypes), name)
        for msg_subtype in self._services:
            if msg_subtype in msg_subtypes:
                print "[Broker] service type %s has been registered by %s" % (msg_subtype, self._services[msg_subtype][1])
                raise Exception
            self._services[msg_subtype] = (ingress, name)
        self._start_list.append(service)
        service.set_send_to_broker(self._send_to_me)

    def register_interface(self, interface):
        name = interface.get_name()
        print "[Broker] register interface %s" % name
        self._interfaces[name] = interface
        self._start_list.append(interface)
        interface.set_send_to_broker(self._send_to_me)

    def _send_to_me(self, address, payload):
        if_name = address[0]
        src_lid = address[1]
        dest_gid, src_gid, msg_type, msg_subtype = utils.extract_mult_proto_header_from_str(payload)
        p = payload[MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET:]

        print "[Broker] receive message from interface(%s) and src lid(%s)" % (if_name, src_lid)

        if not self._no_dump:
            utils.dump([if_name, src_lid, dest_gid, src_gid, msg_type, msg_subtype, p])

        if msg_type == CONST.MULT_PROTO_MSG_TYPE:
            if msg_subtype in CONST.MULT_PROTO_MSG_SUBTYPE_PRIORITY_MAP:
                priority = CONST.MULT_PROTO_MSG_SUBTYPE_PRIORITY_MAP[msg_subtype]
                print "[Broker] an INPUT/OUTPUT message with priority(%d) from interface(%s) and src lid(%s)" % (priority, if_name, src_lid)
        elif msg_type == CONST.MULT_PROTO_MSG_TYPE_OTHER:
            priority = CONST.MULT_PROTO_MSG_OTHER_PRIORITY
            print "[Broker] a FORWARD message with priority(%d) from interface(%s) and src lid(%s)" % (priority, if_name, src_lid)
        else:
            print "[Broker] an UNKNOWN message from interface(%s) and src lid(%s)" % (if_name, src_lid)

        self._ingress.put_nowait((priority, (if_name, src_lid, dest_gid, src_gid, msg_type, mgs_subtype, p)))

    def get_all_interfaces(self):
        return self._interfaces.iteritems # pass out function not the keys and values

    def get_interface(self, name):
        return self._interfaces.get(name, None)

    def start(self):
        self._greenlet = gevent.spawn(self._serve_forever)
        for s in self._start_list:
            s.start()
        print "[Broker] Started"
        gevent.sleep(0) # Make the greenlet start first and return

    def close(self):
        for s in self._start_list:
            s.close()
        self._greenlet.kill()
        print "[Broker] Stopped"

    def _serve_forever(self):
        while True:
            if_name, src_lid, dest_gid, src_gid, msg_type, mgs_subtype, p = self._ingress.get()

            if msg_type == CONST.MULT_PROTO_MSG_TYPE:
                print "[Broker] receive service request from interface(%s) and node lid(%s)" % (if_name, src_lid)

                if (msg_subtype in [CONST.MULT_PROTO_MSG_SUBTYPE_GID_REQ,CONST.MULT_PROTO_MSG_SUBTYPE_GID_ACK]) and dest_gid != MASTER_GID:
                    print "[Broker] Cannot forward GIDREQ/ACK to node gid(%s) other than Master" % str(dest_gid)

                elif msg_subtype == CONST.MULT_PROTO_MSG_SUBTYPE_GID_OFFER:
                    print "[Broker] Cannot handle GIDOFFER message here (RPC does)"

                elif msg_subtype in self._services:
                    ingress, name = self._services[msg_subtype]
                    print "[Broker] deliver message to service %s" % name
                    utils.dump([msg_subtype, p])
                    ingress.put_nowait(((if_name, src_lid), p))

                else:
                    print "[Broker] unknown services type %s" % str(msg_subtype)

            elif msg_type == CONST.MULT_PROTO_MSG_TYPE_OTHER:
                if dest_gid == MASTER_GID:
                    # forwarding to master
                    pass
                else:
                    print "[Broker] Cannot forward OTHER message to node gid(%s) other than Master here (RPC does)" % str(dest_gid)

            gevent.sleep(0)