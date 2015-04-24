import gevent
from gevent import socket

import gtwconfig as CONFIG
import mptnUtils as MPTN
from rpcservice import RPCService
from idservice import IDService

import struct
import logging
import random
logging.basicConfig(level=CONFIG.LOG_LEVEL)
logger = logging.getLogger( __name__ )

ONLY_FROM_TCP_SERVER = 0
ONLY_FROM_TRANSPORT_INTERFACE = 1
VALID_FROM_ALL = 2

class Gateway(object):
    def __init__(self, transport_interface):
        assert transport_interface is not None, "Gateway's transport interface is required"
        self._transport_if = transport_interface
        self._transport_if_name = transport_interface.get_name()
        self._greenlets = None
        self._spawn_handlers = []

        # map from MSGTYPE to a tuple (permission, handler)
        self._protocol_handlers = {}

        self._spawn_handlers.append(self._serve_transport_if_forever)
        self._spawn_handlers.append(self._serve_socket_forever)

        # Initialize AutoNet service
        autonet_mac_address = None
        if CONFIG.ENABLE_AUTONET:
            from autonet import AutoNet
            self._autonet = AutoNet(self._transport_if.get_learn_handlers())
            autonet_mac_address = self._autonet.get_gateway_mac_address()
            self._spawn_handlers.append(self._autonet.serve_autonet)

        # Initialize Monitor Service
        if CONFIG.ENABLE_MONITOR:
            from monitorservice import MonitorService
            self._monitor_service = MonitorService(CONFIG.MONGODB_URL)
            self._monitor_handler = self._monitor_service.handle_monitor_message
            self._spawn_handlers.append(self._monitor_service.serve_monitor)

        # Initialize ID service
        self._id_service = IDService(self._transport_if, autonet_mac_address)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_GWIDACK]  = (ONLY_FROM_TCP_SERVER, MPTN.handle_reply_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_GWIDNAK]  = (ONLY_FROM_TCP_SERVER, MPTN.handle_reply_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_IDREQ]  = (ONLY_FROM_TRANSPORT_INTERFACE, self._id_service.handle_id_req_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_IDACK]  = (ONLY_FROM_TCP_SERVER, MPTN.handle_reply_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_IDNAK]  = (ONLY_FROM_TCP_SERVER, MPTN.handle_reply_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_RTPING] = (ONLY_FROM_TCP_SERVER, self._id_service.handle_rt_ping_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_RTREQ] = (ONLY_FROM_TCP_SERVER, self._id_service.handle_rt_req_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_RTREP] = (ONLY_FROM_TCP_SERVER, MPTN.handle_reply_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_FWDREQ] = (VALID_FROM_ALL, self._id_service.handle_fwd_req_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_FWDACK] = (ONLY_FROM_TCP_SERVER, MPTN.handle_reply_message)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_FWDNAK] = (ONLY_FROM_TCP_SERVER, MPTN.handle_reply_message)

        self._spawn_handlers.append(self._id_service.rt_ping_forever)
        if CONFIG.UNITTEST_MODE:
            self._spawn_handlers.append(self._id_service.clear_id_req_queue)

        # Initialize RPC service
        self._rpc_service = RPCService(self._transport_if.get_rpc_function_lists(), self._id_service)
        self._protocol_handlers[MPTN.MPTN_MSGTYPE_RPCCMD] = (ONLY_FROM_TCP_SERVER, self._rpc_service.handle_rpc_cmd_message)

        # Initialize Ping service
        logger.info("initialized")

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
        gevent.sleep(0.5) # Make the greenlet start first and return

        logger.info("started on %s:%s" % self._tcp_server.getsockname()[:2])
        return True

    def stop(self):
        self._tcp_server.close()

        for g in self._greenlets:
            g.kill()

        logger.warn("stopped")

    def _process_message(self, context, message, from_transport_if):
        NO_RESPONE = None
        if len(message) < MPTN.MPTN_PAYLOAD_BYTE_OFFSET:
            logger.error("receive message with header shorter than required %s" % MPTN.formatted_print(str(message)))
            return NO_RESPONE

        log_msg = MPTN.formatted_print(MPTN.split_packet_to_list(message))
        logger.debug("receives message:\n%s" % log_msg)

        dest_id, src_id, msg_type, payload = MPTN.extract_packet_from_str(message)

        permission, protocol_handler = self._protocol_handlers.get(msg_type, (None, None))
        if protocol_handler is None:
            logger.error("receives unknown message with type %X\n%s" % (msg_type, log_msg))
            return NO_RESPONE

        elif permission == 0 and from_transport_if:
            logger.error("cannot receive a message with type %X from transport interface %s" % (msg_type, self._transport_if_name))
            return NO_RESPONE

        elif permission == 1 and not from_transport_if:
            logger.error("cannot receive a message with type %X from TCP server" % msg_type)
            return NO_RESPONE

        response = protocol_handler(context, dest_id, src_id, msg_type, payload)
        return response, src_id

    def _serve_socket_forever(self):
        '''
        This TCP server would only serve master and other gateways
        '''
        def handle_socket(sock, addr):
            while True:
                message, nonce = MPTN.recv(sock)
                logger.debug("tcp server receives message from address %s" % str(addr))
                if message is None:
                    logger.error("tcp server receive occurs error")
                    MPTN.remove_peer(src_id, addr)
                    return
                response, src_id = self._process_message((sock, addr, nonce), message, False)
                MPTN.add_peer(src_id, addr, sock)
                if response is not None:
                    retries = CONFIG.CONNECTION_RETRIES
                    while retries > 0:
                        ret = MPTN.send(sock, message, nonce)
                        if ret:
                            break
                        retries -= 1
                        gevent.sleep(0.001)
                gevent.sleep(0.001)

        logger.info("TCP server for Master and other Gateways is ready to accept")
        while True:
            socket, address = self._tcp_server.accept()
            gevent.spawn(handle_socket, socket, address)
            gevent.sleep(0.001)

    def _serve_transport_if_forever(self):
        logger.info("Transport interface for MPTN nodes is ready to receive")

        def handle_transport_if(src_addr, message):
            logger.debug("transport interface %s receives message from address %X" % (self._transport_if_name, src_addr))

            response, _ = self._process_message(src_addr, message, True)
            if response is not None:
                response = map(ord, response)
                retries = CONFIG.CONNECTION_RETRIES
                while retries > 0:
                    ret = self._transport_if.send(src_addr, response)
                    if ret[0]: break
                    retries -= 1
                    gevent.sleep(0.001)

        while True:
            src_addr, message = self._transport_if.recv()
            if src_addr is None or message is None: continue
            gevent.spawn(handle_transport_if, src_addr, message)
            gevent.sleep(0.001)