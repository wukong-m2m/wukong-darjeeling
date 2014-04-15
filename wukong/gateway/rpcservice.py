from tinyrpc.protocols.jsonrpc import JSONRPCProtocol
from tinyrpc.dispatch import RPCDispatcher
from tinyrpc.exc import RPCError

import gevent

import gtwconfig as CONFIG
import mptn as MPTN

import logging
logging.basicConfig(level=CONFIG.LOG_LEVEL)
logger = logging.getLogger( __name__ )

class RPCService(object):
    def __init__(self, func_list):
        self._rpc_dispatcher = RPCDispatcher()
        self._rpc_protocol = JSONRPCProtocol()

        for f in func_list:
            self._rpc_dispatcher.add_method(f)

        logger.info("RPC service initialized")

    def handle_message(self, context, dest_did, src_did, msg_type, msg_subtype, payload):
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
        
        return response