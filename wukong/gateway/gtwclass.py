import gevent
from gevent import socket

import gtwconfig as CONFIG
import mptn as MPTN
import utils
from rpcservice import RPCService
from didservice import DIDService

import struct
import logging
logging.basicConfig(level=CONFIG.LOG_LEVEL)
logger = logging.getLogger( __name__ )

class Gateway(object):

    def __init__(self, transport):
        assert transport is not None, "Gateway's transport radio interface is required"
        self._transport = transport

        self._greenlet = None
        self._no_dump = False

        function_lists = self._transport_function_init()
        self._rpc_service = RPCService(function_lists)
        self._rpc_handler = self._rpc_service.handle_message

        self._did_service = DIDService(self._transport.get_radio_address(), self._transport.get_radio_addr_len())
        self._did_req_handler = self._did_service.handle_did_req_message
        self._fwd_handler = self._did_service.handle_fwd_message
        self._pfx_upd_handler = self._did_service.handle_pfx_upd_message

        logger.info("initialized")

    def start(self, tcp_port):
        try:
            self._ip_server = socket.socket()
            self._ip_server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self._ip_server.bind(('', tcp_port))
            self._ip_server.listen(500)
        except socket.error as msg:
            self._ip_server.close()
            logger.error("cannot start due to socket error: %s" % msg)
            return False
        self._greenlet = [gevent.spawn(self._serve_transport_forever), gevent.spawn(self._serve_socket_forever)]
        gevent.sleep(0) # Make the greenlet start first and return
        logger.info("started on %s:%s" % self._ip_server.getsockname()[:2])
        return True

    def stop(self):
        self._ip_server.close()

        for g in self._greenlet:
            g.kill()
        logger.warn("stopped")

    def _process_message(self, context, message, from_transport):
        if len(message) < MPTN.MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET:
            logger.error("receive message with header shorter than required %s" % utils.formatted_print(str(message)))
            return None

        dest_did, src_did, msg_type, msg_subtype = utils.extract_mult_proto_header_from_str(message)
        payload = message[MPTN.MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET:] if len(message) > MPTN.MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET else None

        if isinstance(context, tuple) and len(context) == 2:
            context_str = ':'.join([str(i) for i in context])
        else:
            context_str = str(context)

        log_msg = [context_str] + utils.split_packet_header(message)
        if payload is not None: log_msg.append(payload)
        log_msg = utils.formatted_print(log_msg)
        if msg_type == MPTN.MULT_PROTO_MSG_TYPE_DID:
            # Only DID REQ message from transport interface would get here
            if not from_transport:
                logger.error("cannot receive DID message from TCP server")
                return None

            if msg_subtype != MPTN.MULT_PROTO_MSG_SUBTYPE_DID_REQ:
                logger.error("receives invalid DID message subtype %X and will not send any reply" % msg_subtype)
                return None

            if dest_did != 0 and src_did != 0xFFFFFFFF:
                logger.error("DID REQ with incorrect src did = %X and/or dest did = %X" % (src_did, dest_did))
                return None

            logger.debug("receive DID REQ message:\n" + log_msg)
            message = self._did_req_handler(context, dest_did, src_did, msg_type, msg_subtype, payload)
            if message is None:
                return None

            return message

        # For the rest of msg_type
        # Check if SRC_DID is on MPTN network
        # Also check if DEST_DID is on MPTN network
        # Drop if one of test is not passed
        if not self._did_service.is_did_valid(dest_did):
            logger.debug("drop message since DEST DID (%X) is not valid" % dest_did)
            return None

        if not self._did_service.is_did_valid(src_did):
            logger.debug("drop message since SRC DID (%X) is not valid" % src_did)
            return None

        if msg_type == MPTN.MULT_PROTO_MSG_TYPE_RAD:
            # Only RAD message from external service would get here
            if from_transport:
                logger.error("cannot receive RAD message from TCP server")
                return None

            if payload is not None:
                logger.error("packet RAD might not have the payload")
                return None

            if msg_subtype in self._transport_learn_handler:
                logger.debug("receive RAD message:\n" + log_msg)
                success = self._transport_learn_handler[msg_subtype]()
                if success:
                    msg_subtype = MPTN.MULT_PROTO_MSG_SUBTYPE_RAD_ACK
                else:
                    msg_subtype = MPTN.MULT_PROTO_MSG_SUBTYPE_RAD_NAK
                payload = str(success)

            elif msg_subtype == MPTN.MULT_PROTO_MSG_SUBTYPE_RAD_POLL:
                logger.debug("receive RAD POLL message:\n" + log_msg)
                payload = str(self._transport_poll_handler())
                msg_subtype = MPTN.MULT_PROTO_MSG_SUBTYPE_RAD_ACK

            else:
                logger.error("receives invalid RAD message subtype and will not send any reply")
                return None

            header = utils.create_mult_proto_header_to_str(src_did, dest_did, msg_type, msg_subtype)
            return header + payload

        elif msg_type == MPTN.MULT_PROTO_MSG_TYPE_RPC:
            # Only RPC CMD message from master would get here
            # Check if SRC_DID is master's
            # RPC handler would reply a full MPTN packet
            if from_transport:
                logger.error("cannot receive RPC message from transport radio interface")
                return None

            if payload is None:
                logger.error("packet RPC might have the payload")
                return None

            if msg_subtype != MPTN.MULT_PROTO_MSG_SUBTYPE_RPC_CMD:
                logger.error("receives invalid RPC message subtype and will not send any reply")
                return None

            logger.debug("receive RPC CMD message:\n" + log_msg)
            header = utils.create_mult_proto_header_to_str(src_did, dest_did, msg_type, MPTN.MULT_PROTO_MSG_SUBTYPE_RPC_REP)
            payload = self._rpc_handler(context, dest_did, src_did, msg_type, msg_subtype, payload)
            return header + payload

        elif msg_type == MPTN.MULT_PROTO_MSG_TYPE_FWD:
            # Check if DEST_DID is on the same network and get radio address of it
            #   True:   send to transport and return back to source gateway or master
            #   False:  find and then send to radio address of destination IP hop
            if payload is None:
                logger.error("packet FWD might have the payload")
                return None

            if msg_subtype != MPTN.MULT_PROTO_MSG_SUBTYPE_FWD:
                logger.error("receives invalid FWD FWD message subtype and will not send any reply")
                return None

            logger.debug("receive FWD FWD message:\n" + log_msg)
            radio_address = self._fwd_handler(context, dest_did, message)
            if radio_address is None:
                logger.debug("the forward destination DID is not in this gateway's network")
                return None

            retries = CONFIG.CONNECTION_RETRIES
            for i in xrange(retries):
                success, message = self._transport_send_handler(radio_address, message)
                if success:
                    msg_subtype = MPTN.MULT_PROTO_MSG_SUBTYPE_FWD_ACK
                    header = utils.create_mult_proto_header_to_str(src_did, dest_did, msg_type, msg_subtype)
                    return header
                gevent.sleep(0)
            else:
                header = utils.create_mult_proto_header_to_str(src_did, dest_did, msg_type, MPTN.MULT_PROTO_MSG_SUBTYPE_FWD_NAK)
                logger.error("forwarding message to radio address %X fail" % radio_address)
                return header + message

        elif msg_type == MPTN.MULT_PROTO_MSG_TYPE_PFX:
            # Only PFX UPD message from master would get here
            if from_transport:
                logger.error("cannot receive RPC message from transport radio interface")
                return None

            if msg_subtype != MPTN.MULT_PROTO_MSG_SUBTYPE_PFX_UPD:
                logger.error("receives message subtype other than PFX UPD and will not send any reply")
                return None

            if payload is None:
                logger.error("packet PFX might have the payload")
                return None

            logger.debug("receive PFX UPD message:\n" + log_msg)
            self._pfx_upd_handler(context, dest_did, src_did, msg_type, msg_subtype, payload)
            return None

        else:
            logger.error("receives unknown message with type %X\n%s" % (msg_type, log_msg))
            return None

    def _serve_socket_forever(self):
        '''
        This TCP server would only serve master and other gateways
        '''
        def handle_socket(sock, addr):
            try:
                logger.debug("inner TCP server accepts connection from address %s" % str(addr))
                sock.settimeout(CONFIG.NETWORK_TIMEOUT)
                message = utils.special_recv(sock)
                response = self._process_message(addr, message, False)
                if response is not None: utils.special_send(sock, response)
            except socket.timeout:
                logger.error("the socket is timeout from addr=%s with msg=%s" % (str(addr),message))
            except socket.error as e:
                logger.error("gets socket error %s from addr=%s with msg=%s" % (str(e), str(addr), message))
            except struct.error as e:
                logger.error("python struct cannot interpret message %s" % message)
            finally:
                sock.close()

        logger.info("inner TCP server is ready to accept")
        while True:
            new_sock, address = self._ip_server.accept()
            gevent.spawn(handle_socket, new_sock, address)
            gevent.sleep(0)

    def _serve_transport_forever(self):
        logger.info("inner transport radio interface is ready to receive")
        while True:
            src_radio_addr, message = self._transport.recv()

            if src_radio_addr is not None and message is not None:
                def handle_transport(src_radio_addr, message):
                    logger.debug("transport radio interface receives message from address %X" % src_radio_addr)
                    response = self._process_message(src_radio_addr, message, True)
                    if response is not None:
                        response = [0x88] + map(ord, response)
                        retries = CONFIG.CONNECTION_RETRIES
                        while retries > 0:
                            ret = self._transport_send_handler(src_radio_addr, response)
                            if ret[0]:
                                logger.debug("transport radio interface has sent message %s to address %X" % (str(response), src_radio_addr))
                                break
                            retries -= 1
                            logger.debug("transport radio interface fails to send message %s to address %X" % (str(response), src_radio_addr))
                            gevent.sleep(0.01)
                gevent.spawn(handle_transport, src_radio_addr, message)
            gevent.sleep(0.01)

    def _transport_function_init(self):

        def send(radio_address, message):
            e = self._transport.send(radio_address, message)
            if e is None:
                return (True, None)

            msg = "transport radio interface(%s) fails to send message to device with radio address(%s) and error: %s\n\tmsg: %s" % (self._transport.get_name(), str(radio_address), e, message)
            logger.warning(msg)
            return (False, msg)

        def getDeviceType(radio_address):
            return self._transport.get_device_type(radio_address)

        def routing():
            r = {}
            for node in self._transport.discover():
                r[node] = self._transport.routing(node)
            return r

        def discover():
            return self._transport.discover()

        def poll():
            return self._transport.poll()

        def add():
            r = self._transport.add_mode()
            if r != True:
                logger.error("fails to enable inclusion mode of interface(%s)" % self._transport.get_name())
            return r

        def delete():
            r = self._transport.delete_mode()
            if r != True:
                logger.error("fails to enable exclusion mode of interface(%s)" % self._transport.get_name())
            return r

        def stop():
            r = self._transport.stop_mode()
            if r != True:
                logger.error("fails to stop learning mode of interface(%s)" % self._transport.get_name())
            return r

        self._transport_send_handler = send
        self._transport_learn_handler = {}
        self._transport_learn_handler[MPTN.MULT_PROTO_MSG_SUBTYPE_RAD_ADD] = add
        self._transport_learn_handler[MPTN.MULT_PROTO_MSG_SUBTYPE_RAD_DEL] = delete
        self._transport_learn_handler[MPTN.MULT_PROTO_MSG_SUBTYPE_RAD_STOP] = stop
        self._transport_poll_handler = poll
        function_lists = [send, getDeviceType, routing, discover, add, delete, stop, poll]
        return function_lists