from tinyrpc.protocols.jsonrpc import JSONRPCProtocol
from tinyrpc.dispatch import RPCDispatcher
from tinyrpc.exc import RPCError

import gevent

import gtwconfig as CONFIG
import mptnUtils as MPTN

import color_logging, logging
logger = logging

class RPCService(object):
    def __init__(self, function_lists, is_id_valid):
        self._rpc_dispatcher = RPCDispatcher()
        self._rpc_protocol = JSONRPCProtocol()
        self._is_id_valid = is_id_valid

        for f in function_lists:
            self._rpc_dispatcher.add_method(f)

        logger.info("RPC service initialized")

    def handle_rpccmd_message(self, context, dest_id, src_id, msg_type, payload):
        # logger.debug("got RPCCMD message from src_id %s:\n%s" % (MPTN.ID_TO_STRING(src_id), MPTN.formatted_print([payload])))
        if not self._is_id_valid(dest_id):
            logger.error("invalid RPCCMD dest ID %X: not found in network" % dest_id)
            return None

        if src_id != MPTN.MASTER_ID:
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

        message = MPTN.create_packet_to_str(src_id, dest_id, MPTN.MPTN_MSGTYPE_RPCREP, response)
        MPTN.socket_send(context, src_id, message)
        return