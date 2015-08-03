import gevent
from gevent import socket

import gtwconfig as CONFIG
import mptnUtils as MPTN
from rpcservice import RPCService
from idservice import IDService

import struct
import color_logging, logging
logger = logging
import random

class Gateway(object):
    def __init__(self, transport_interface):
        self._greenlets = None
        self._spawn_handlers = []

        assert transport_interface is not None, "Gateway's transport interface is required"
        self._transport_if = transport_interface
        self._transport_if_name = transport_interface.get_name()
        MPTN.set_transport_if_send(self._transport_if_retried_send)

        # map from MSGTYPE to a tuple (permission, handler)
        self._protocol_handlers = {}
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_GWIDACK] = MPTN.ProtocolHandler(MPTN.ONLY_FROM_TCP_SERVER, MPTN.handle_reply_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_GWIDNAK] = MPTN.ProtocolHandler(MPTN.ONLY_FROM_TCP_SERVER, MPTN.handle_reply_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_IDACK] = MPTN.ProtocolHandler(MPTN.ONLY_FROM_TCP_SERVER, MPTN.handle_reply_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_IDNAK] = MPTN.ProtocolHandler(MPTN.ONLY_FROM_TCP_SERVER, MPTN.handle_reply_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_FWDACK] = MPTN.ProtocolHandler(MPTN.ONLY_FROM_TCP_SERVER, MPTN.handle_reply_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_FWDNAK] = MPTN.ProtocolHandler(MPTN.ONLY_FROM_TCP_SERVER, MPTN.handle_reply_message)
        MPTN.set_message_handler(self._process_message)

        self._spawn_handlers.append(self._serve_socket_forever)
        self._spawn_handlers.append(self._serve_transport_if_forever)

        gateway_application_handlers = {}
        # Initialize AutoNet service
        autonet_mac_address = []
        if CONFIG.ENABLE_AUTONET:
            from autonet import AutoNet
            self._autonet = AutoNet(self._transport_if.get_learn_handlers())
            autonet_mac_address = self._autonet.get_gateway_mac_address()
            self._spawn_handlers.append(self._autonet.serve_autonet)

        # Initialize Monitor Service
        if CONFIG.ENABLE_MONITOR:
            from monitorservice import MonitorService
            self._monitor_service = MonitorService(CONFIG.MONGODB_URL)
            self._spawn_handlers.append(self._monitor_service.serve_monitor)
            gateway_application_handlers[MPTN.WKPF_COMMAND_MONITOR] = self._monitor_service.handle_monitor_message

        # Initialize ID service
        self._id_service = IDService(self._transport_if.get_address(), self._transport_if.get_addr_len(), autonet_mac_address, gateway_application_handlers)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_IDREQ] = MPTN.ProtocolHandler(MPTN.ONLY_FROM_TRANSPORT_INTERFACE, self._id_service.handle_idreq_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_RTPING] = MPTN.ProtocolHandler(MPTN.ONLY_FROM_TCP_SERVER, self._id_service.handle_rtping_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_RTREQ] = MPTN.ProtocolHandler(MPTN.ONLY_FROM_TCP_SERVER, self._id_service.handle_rtreq_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_RTREP] = MPTN.ProtocolHandler(MPTN.ONLY_FROM_TCP_SERVER, self._id_service.handle_rtrep_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_FWDREQ] = MPTN.ProtocolHandler(MPTN.VALID_FROM_ALL, self._id_service.handle_fwdreq_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_GWDISCOVER] = MPTN.ProtocolHandler(MPTN.VALID_FROM_ALL, self._id_service.handle_gwdiscover_message)

        self._spawn_handlers.append(self._id_service.rtping_forever)
        if CONFIG.UNITTEST_MODE:
            self._spawn_handlers.append(self._id_service.clear_id_req_queue)

        # Initialize RPC service
        self._rpc_service = RPCService(self._transport_if.get_rpc_function_lists(), self._id_service.is_id_valid)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_RPCCMD] = MPTN.ProtocolHandler(MPTN.ONLY_FROM_TCP_SERVER, self._rpc_service.handle_rpccmd_message)

        # Initialize Ping service
        # logger.info("All service initialized. ready to spawn greenlets")

    def start(self, tcp_port):
        try:
            self._tcp_server = socket.socket()
            self._tcp_server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self._tcp_server.bind(('', tcp_port))
            self._tcp_server.listen(100)
        except socket.error as msg:
            self._tcp_server.close()
            logger.error("cannot start due to socket error: %s" % msg)
            return False
        self._greenlets = [gevent.spawn(f) for f in self._spawn_handlers]
        gevent.sleep(0) # Make the greenlet start first and return

        logger.info("All greenlets are spawn and TCP server listens on %s:%s" % self._tcp_server.getsockname()[:2])
        return True

    def stop(self):
        self._tcp_server.close()

        for g in self._greenlets: g.kill()

        logger.warn("stopped")

    def _process_message(self, context, message):
        dest_id, src_id, msg_type, payload = MPTN.extract_packet_from_str(message)

        log_msg = MPTN.formatted_print(MPTN.split_packet_to_list(message))
        # logger.debug("receives and processes message:\n%s" % log_msg)

        if dest_id is None:
            logger.error("processing message with header shorter than required %s" % MPTN.formatted_print(str(message)))
            return

        protocol_handler = self._protocol_handlers.get(msg_type)
        if protocol_handler is None:
            logger.error("processing unknown message with type %X\n%s" % (msg_type, log_msg))
            return

        permission = protocol_handler.permission
        if (permission & context.direction) == 0:
            if permission == MPTN.ONLY_FROM_TCP_SERVER:
                logger.error("cannot process a message with type %X from transport interface %s" % (msg_type, self._transport_if_name))
                return

            elif permission == MPTN.ONLY_FROM_TRANSPORT_INTERFACE:
                logger.error("cannot process a message with type %X from TCP server" % msg_type)
                return

        protocol_handler.handler(context, dest_id, src_id, msg_type, payload)
        return

    def _serve_socket_forever(self):
        '''
        This TCP server would only serve master and other gateways
        '''
        # logger.info("TCP server for Master and other Gateways is ready to accept")
        while True:
            new_sock, address = self._tcp_server.accept()
            logger.info("TCP Server accepts from address %s" % str(address))
            gevent.spawn(MPTN.handle_socket, new_sock, address)
            gevent.sleep(0)

    def _serve_transport_if_forever(self):
        def handle_transport_if(src_addr, message):
            logger.debug("transport interface %s receives message from address %X" % (self._transport_if_name, src_addr))
            self._process_message(MPTN.new_if_context(src_addr), message)

        # logger.info("Transport interface for MPTN nodes is ready to receive")
        while True:
            src_addr, message = self._transport_if.recv()
            if src_addr is not None and message is not None:
                gevent.spawn(handle_transport_if, src_addr, message)
            gevent.sleep(0.01)

    def _transport_if_retried_send(self, dest_addr, message):
        retries = MPTN.CONNECTION_RETRIES
        message = map(ord, message)
        for i in xrange(retries):
            ret = self._transport_if.send(dest_addr, message)
            if ret[0]: return ret
            gevent.sleep(0.01)
