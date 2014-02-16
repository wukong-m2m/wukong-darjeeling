import gevent
from gevent.queue import Queue

import gtwconfig as CONST
import utils

class Broker(object):

    ADD_MODE = 1
    DEL_MODE = 2
    STOP_MODE = 3

    def __init__(self):
        self.mode = Broker.STOP_MODE
        self.interfaces = {}
        self.protocols = {}
        self.services = {}
        self.ingress = Queue()
        self._greenlet = None
        self.no_dump = False
        print "[Broker] initialized"
    
    def registerService(self, name, ingress, msg_subtypes):
        print "[Broker] register service types %s under name '%s'" % (str(msg_subtypes), name)
        #self.services[name] = ingress
        for msg_subtype in self.services:
            if msg_subtype in msg_subtypes:
                print "[Broker] service type %s has been registered by %s" % (msg_subtype, self.services[msg_subtype][1])
                raise Exception
            self.services[msg_subtype] = (ingress, name)
        return self.ingress
    
    def registerInterface(self, name, interface, if_protocol):
        print "[Broker] register interface %s" % name
        self.interfaces[name] = interface
        if if_protocol not in self.protocols:
            self.protocols[if_protocol] = []
        self.protocols[if_protocol].append(name)
        return self.ingress

    def getAllInterfaces(self):
        return self.interfaces.iteritems()

    def getInterface(self, name):
        return self.interfaces[name]

    def nodeInclusion(self):
        if self.mode != Broker.STOP_MODE:
            return False

        success_list = []
        for n, i in self.getAllInterfaces():
            try:
                i.add()
                success_list.append(i)
            except:
                print "[Broker] node inclusion error from interface %s" % n

        if len(success_list):
            self.mode = Broker.ADD_MODE
            return True

        return False

    def nodeExclusion(self):
        if self.mode != Broker.STOP_MODE:
            return False

        success_list = []
        for n, i in self.getAllInterfaces():
            try:
                i.delete()
                success_list.append(i)
            except:
                print "[Broker] node exclusion error from interface %s" % n

        if len(success_list):
            self.mode = Broker.DEL_MODE
            return True

        return False

    def stopInEx(self):
        success_list = []
        for n, i in self.getAllInterfaces():
            try:
                i.stop()
                success_list.append(i)
            except:
                print "[Broker] stop incl./excl. error from interface %s but continue" % n

        if len(success_list) == len(self.interfaces):
            self.mode = Broker.STOP_MODE
            return True

        return False

    def start(self):
        print "[Broker] Started"
        self._greenlet = gevent.spawn(self.serve_forever)
        gevent.sleep(0) # Make the greenlet start first and return

    def close(self):
        print "[Broker] Stop working"
        self._greenlet.kill()

    def serve_forever(self):
        while True:
            address, callback_queue, payload = self.ingress.get()

            msg_type = payload[CONST.MULT_PROTO_MSG_TYPE_BYTE_OFFSET]
            msg_subtype = payload[CONST.MULT_PROTO_MSG_SUBTYPE_BYTE_OFFSET]
            p = payload[CONST.MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET:]
            
            if msg_type == CONST.MULT_PROTO_MSG_TYPE:
                print "[Broker] receive service request from %s" % str(address)

                if msg_subtype in [CONST.MULT_PROTO_MSG_SUBTYPE_GID_REQ,CONST.MULT_PROTO_MSG_SUBTYPE_GID_ACK]:
                    if dest_gid == MASTER_GID:
                        # forwarding to master
                        pass
                    else:
                        print "[Broker] invalid GID REQ/ACK message"

                elif msg_subtype == CONST.MULT_PROTO_MSG_SUBTYPE_GID_OFFER:
                    print "[Broker] should not handle GID OFFER message here"

                elif msg_subtype in self.services:
                    ingress, name = self.services[msg_subtype]
                    print "[Broker] deliver service to %s" % name
                    utils.dump([msg_subtype, p])
                    ingress.put_nowait((address, callback_queue, p))
                
                else:
                    print "[Broker] unknown services type %s" % hex(msg_subtype)

            elif msg_type == CONST.MULT_PROTO_MSG_TYPE_APP:
                dest_gid = payload[CONST.MULT_PROTO_DEST_BYTE_OFFSET:CONST.MULT_PROTO_DEST_BYTE_OFFSET+CONST.MULT_PROTO_LEN_GID]
                if dest_gid == MASTER_GID:
                    # forwarding to master
                    pass
                else:
                    print "[Broker] should not handle APP message to node here"
            else:
                print "[Broker] receive unknown message from %s" % str(address)
                
            if not self.no_dump:
                i = CONST.MULT_PROTO_DEST_BYTE_OFFSET
                dest_gid = payload[i:i+CONST.MULT_PROTO_LEN_GID]
                i = CONST.MULT_PROTO_SRC_BYTE_OFFSET
                src_gid = payload[i:i+CONST.MULT_PROTO_LEN_GID]
                utils.dump([dest_gid, src_gid, [msg_type], [msg_subtype], p])

            gevent.sleep(0)