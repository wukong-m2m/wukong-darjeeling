from tinyrpc.protocols.jsonrpc import JSONRPCProtocol
from tinyrpc.dispatch import RPCDispatcher
from tinyrpc.exc import RPCError

import gevent

import gtwconfig as CONFIG
import mptnUtils as MPTN

import logging
logging.basicConfig(level=CONFIG.LOG_LEVEL)
logger = logging.getLogger( __name__ )

class RPCService(object):
    def __init__(self, function_lists, id_service):
        self._rpc_dispatcher = RPCDispatcher()
        self._rpc_protocol = JSONRPCProtocol()
        self._is_id_valid = id_service.is_id_valid
        self._is_id_master = id_service.is_id_master

        for f in function_lists:
            self._rpc_dispatcher.add_method(f)

        logger.info("RPC service initialized")

    def handle_rpc_cmd_message(self, context, dest_id, src_id, msg_type, payload):
        if self._is_id_valid(dest_id):
            logger.error("invalid RPCCMD dest ID %X: not found in network" % dest_id)
            return None

        if self._is_id_master(src_id):
            logger.error("invalid RPCCMD src ID %X: should be master" % src_id)
            return None

        if payload is None:
            logger.error("packet RPCCMD should have the payload")
            return None

        try:
            request = self._rpc_protocol.parse_request(payload)
        except RPCError as e:
            response = e.error_respond()
        else:
            response = self._rpc_dispatcher.dispatch(request)

        if response is None:
            response = str(None)
        else:
            response = response.serialize()

        return MPTN.create_packet_to_str(src_id, dest_id, MPTN.MPTN_MSGTYPE_RPCREP, response)