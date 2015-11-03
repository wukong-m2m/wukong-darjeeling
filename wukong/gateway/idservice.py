# try:
#     import netifaces
# except:
#     print "Please install the netifaces module from pypi"
#     print "e.g. sudo pip install netifaces"
#     exit(-1)

# try:
#     import pytrie
# except:
#     print "Please install the netifaces module from pypi"
#     print "e.g. sudo pip install pytrie"
#     exit(-1)

import gtwconfig as CONFIG
import mptnUtils as MPTN
from mptnUtils import DBDict

import gevent
from gevent import socket
from gevent.queue import Queue
import struct
from datetime import datetime
import uuid
import json
import hashlib
from sets import Set
import traceback
import color_logging, logging
logger = logging

RANDOM_BYTES = map(ord, uuid.uuid4().bytes)

class IDService(object):
    def __init__(self, transport_if_addr, transport_if_len, transport_if_send, autonet_mac_addr=[], gateway_application_handlers={}):
        self._app_handler = gateway_application_handlers
        self._transport_if_addr = transport_if_addr
        self._transport_if_addr_len = transport_if_len
        self._transport_if_send = transport_if_send

        # _nexthop_db: key = "MPTN ID/NETMASK" STRING, value = next hop's tcp_address tuple ("IP" STRING, PORT INT)
        self._nexthop_db = DBDict("gtw_nexthop_table.sqlite")
        self._init_nexthop_lookup()

        # _addr_db: key = address, value = UUID (such as MAC address)
        self._addr_db = DBDict("gtw_addr_uuid_table.sqlite")

        self._settings_db = DBDict("gtw_settings_db.sqlite")
        self._init_settings_db(autonet_mac_addr)

        if CONFIG.UNITTEST_MODE: self._id_req_queue = Queue()

        logger.info("IDService initialized with gateway ID is %s 0x%X" % (MPTN.ID_TO_STRING(self._settings_db["GTWSELF_ID"]),self._settings_db["GTWSELF_ID"]))

    def _init_settings_db(self, autonet_mac_addr=RANDOM_BYTES):
        if self._transport_if_addr_len > MPTN.MPTN_ID_LEN:
            logger.error("_init_settings_db length of address (%d) is too long to support" % self._transport_if_addr_len)
            self._clear_settings_db()
            exit(-1)

        if self._transport_if_addr_len == 1 or self._transport_if_addr_len == 2:
            assert isinstance(self._transport_if_addr, int) and (self._transport_if_addr < 2 ** (self._transport_if_addr_len*8)), "_init_settings_db ZW or ZB interface address must a integer (%s) with max %d" % (str(self._transport_if_addr), self._transport_if_addr_len)

            interface = MPTN.ID_INTERFACE_FROM_TUPLE(
                MPTN.ID_TO_STRING(self._transport_if_addr),
                str((MPTN.MPTN_ID_LEN-self._transport_if_addr_len)*8)
            )

        elif self._transport_if_addr_len == 4:
            assert isinstance(self._transport_if_addr, tuple) and (len(self._transport_if_addr) == 2) and isinstance(self._transport_if_addr[0], basestring) and isinstance(self._transport_if_addr[1], basestring), "_init_settings_db UDP interface address must be a tuple with 2 strings ('IP', 'NETMASK') where NETMASK could be either a single number or or a string representation"

            interface = MPTN.ID_INTERFACE_FROM_TUPLE(self._transport_if_addr[0], self._transport_if_addr[1])

        else:
            logger.error("_init_settings_db Unsupported interface type: %s" % CONFIG.TRANSPORT_INTERFACE_TYPE)
            self._clear_settings_db()
            exit(-1)

        self._transport_if_addr = int(interface.ip)
        self._transport_network_size = interface.network.num_addresses
        self._id_prefix_len = interface.network.prefixlen
        self._id_hostmask = int(interface.network.hostmask) if (self._transport_if_addr_len == 2 or self._transport_if_addr_len == 1) else MPTN.MPTN_MAX_ID
        self._id_netmask = int(interface.network.netmask)

        if "GTWSELF_UNIQUE_VALUE" not in self._settings_db:
            if len(autonet_mac_addr) < MPTN.GWIDREQ_PAYLOAD_LEN:
                autonet_mac_addr = RANDOM_BYTES[0:MPTN.GWIDREQ_PAYLOAD_LEN-len(autonet_mac_addr)] + autonet_mac_addr
            self._settings_db["GTWSELF_UNIQUE_VALUE"] = autonet_mac_addr
            logger.debug("GTWSELF_UNIQUE_VALUE is %s" % str(autonet_mac_addr))

        # Check whether master is online and gateway's own "MPTN ID/prefix" is valid or not
        self._init_get_prefix_from_master()
        if self._id == MPTN.MPTN_MAX_ID:
            logger.error("_init_settings_db cannot initialize gateway because of no valid ID")
            self._clear_settings_db()
            exit(-1)

        try:
            self._nexthop_lookup.pop("%s/%d"%(MPTN.ID_TO_STRING(self._id), self._id_prefix_len))
        except Exception as e:
            pass
        return

    def _clear_settings_db(self):
        for d in [self._settings_db, self._addr_db, self._nexthop_db, self._uuid_db]:
            d.clear()

    def _init_get_prefix_from_master(self):
        if "GTWSELF_ID" not in self._settings_db: self._settings_db["GTWSELF_ID"] = MPTN.MPTN_MAX_ID
        self._id = self._settings_db["GTWSELF_ID"]
        MPTN.set_self_id(self._id)
        if self._id != MPTN.MPTN_MAX_ID:
            self._network = MPTN.ID_NETWORK_FROM_TUPLE(MPTN.ID_TO_STRING(self._id),str(self._id_prefix_len))
            self._id_prefix = self._id & self._id_netmask

        dest_id = MPTN.MASTER_ID
        src_id = self._id
        msg_type = MPTN.MPTN_MSGTYPE_GWIDREQ

        payload = json.dumps({"IFADDR":self._transport_if_addr,
            "IFADDRLEN":self._transport_if_addr_len,
            "IFNETMASK":self._id_netmask, "PORT":CONFIG.SELF_TCP_SERVER_PORT,
            "UUID":self._settings_db["GTWSELF_UNIQUE_VALUE"]})
        # payload = "IFADDR=%d;IFADDRLEN=%d;IFNETMASK=%d;PORT=%d;VAL=%s" % (self._transport_if_addr,
        #     self._transport_if_addr_len, self._id_netmask, CONFIG.SELF_TCP_SERVER_PORT,
        #     struct.pack("!%dB"%MPTN.GWIDREQ_PAYLOAD_LEN, *self._settings_db["GTWSELF_UNIQUE_VALUE"]))
        message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, payload)

        packet = MPTN.socket_send(None, MPTN.MASTER_ID, message, expect_reply=True)
        if packet is None:
            logger.error("GWIDREQ cannot get GWIDACK/NAK from master due to network problem")
            return None

        # log_msg = MPTN.formatted_print(MPTN.split_packet_to_list(message))
        dest_id, src_id, msg_type, payload = packet

        if msg_type == MPTN.MPTN_MSGTYPE_GWIDNAK:
            logger.error("GWIDREQ GWIDREQ is refused by master")
            return None

        elif msg_type != MPTN.MPTN_MSGTYPE_GWIDACK:
            logger.error("GWIDREQ get unexpected msg type (%d) instead of GWIDACK/NAK from master" % msg_type)
            return None

        elif dest_id != self._id:
            if self._id == MPTN.MPTN_MAX_ID:
                self._settings_db["GTWSELF_ID"] = dest_id
                self._id = dest_id
                MPTN.set_self_id(self._id)
                self._network = MPTN.ID_NETWORK_FROM_TUPLE(MPTN.ID_TO_STRING(self._id),str(self._id_prefix_len))
                self._id_prefix = dest_id & self._id_netmask
                logger.info("GWIDREQ successfully get new ID %s including prefix" % MPTN.ID_TO_STRING(dest_id))
            else:
                logger.error("GWIDREQ get an ID %d %s different from old one %d %s" % (dest_id, MPTN.ID_TO_STRING(dest_id), self._id, MPTN.ID_TO_STRING(self._id)))
                exit(-1)
        else:
            logger.info("GWIDREQ successfully check the ID %s with master"% MPTN.ID_TO_STRING(dest_id))

    def _init_nexthop_lookup(self):
        self._nexthop_lookup = {MPTN.ID_NETWORK_FROM_STRING(network_string):MPTN.NextHop(id=MPTN.ID_FROM_STRING(network_string),tcp_address=tcp_address) for (network_string, tcp_address) in self._nexthop_db.iteritems()}
        MPTN.set_nexthop_lookup_function(self._find_nexthop_for_id)
        self._update_nexthop_hash()

    def _update_nexthop_hash(self):
        j = "EmPtY"
        nexthop_db = {key:value for (key,value) in self._nexthop_db.iteritems()}
        if len(nexthop_db) > 0:
            j = json.dumps(nexthop_db, sort_keys=True)
        h = hashlib.sha512()
        h.update(j)
        self._nexthop_hash = h.digest()

    def _alloc_address(self, address, uuid):
        assert isinstance(address, (int, long)), "_alloc_address %s must be integer instead of %s" % (str(address), type(address))
        assert MPTN.IS_ID_IN_NETWORK(address, self._network), "_alloc_address %s cannot excede network %s" % (MPTN.ID_TO_STRING(address), str(self._network))
        self._addr_db[address] = uuid

    def _dealloc_address(self, address):
        if self.is_address_valid(address):
            del self._addr_db[address]

    def _get_address_from_id(self, mptn_id):
        return mptn_id & self._id_hostmask

    def _is_id_master(self, mptn_id):
        return mptn_id == MPTN.MASTER_ID

    def _is_id_gwself(self, mptn_id):
        return (mptn_id == self._id) or (mptn_id == 1)

    def _is_id_in_gwself_network(self, mptn_id):
        if MPTN.IS_ID_IN_NETWORK(mptn_id, self._network):
            return self.is_addr_valid(self._get_address_from_id(mptn_id))
        return False

    def _find_nexthop_for_id(self, mptn_id):
        if mptn_id is None:
            return None
        elif self._is_id_master(mptn_id):
            return MPTN.NextHop(id=mptn_id, tcp_address=CONFIG.MASTER_ADDRESS)

        for network, next_hop in self._nexthop_lookup.iteritems():
            if MPTN.IS_ID_IN_NETWORK(mptn_id, network):
                return next_hop
            gevent.sleep(0.001)

        return None

    def _forward_to_next_hop(self, context, dest_id, message):
        packet = MPTN.socket_send(context, dest_id, message, expect_reply=True)

        if packet is None:
            log_msg = MPTN.formatted_print(MPTN.split_packet_to_list(message))
            logger.error("_forward_to_next_hop cannot forward packet to %s due to network problem\n%s" % (str(dest_id), log_msg))
            return False

        # log_msg = MPTN.formatted_print(MPTN.split_packet_to_list(packet))
        dest_id, src_id, msg_type, payload = packet

        if payload is None:
            logger.error("_forward_to_next_hop FWDACK/NAK from master might have the payload")
            return False

        if msg_type == MPTN.MPTN_MSGTYPE_FWDNAK:
            logger.error("_forward_to_next_hop forward via %s fails with error %s" % (str(dest_id), payload))
            return False

        if msg_type != MPTN.MPTN_MSGTYPE_FWDACK:
            logger.error("_forward_to_next_hop get unexpected msg type (%d) instead of FWDACK/NAK" % msg_type)
            return False

        # TODO: may need retransmittion? Could be redundant since WKPF has it.
        return True

    '''
    Public functions
    '''
    def update_addr_db(self, discovered_nodes):
        addr_db_set = Set(map(int, self._addr_db.keys()))
        discovered_nodes_set = Set(map(lambda x: self._id_prefix | x, discovered_nodes))
        to_remove_nodes = list(addr_db_set - discovered_nodes_set)
        if len(to_remove_nodes) == 0: return
        
        for to_remove_node_addr in to_remove_nodes:
            logger.debug("=============Remove not found existed node 0x%X" % (to_remove_node_addr))
            del self._addr_db[to_remove_node_addr]

    def is_addr_valid(self, addr):
        return addr in self._addr_db

    def is_id_valid(self, mptn_id):
        if self._is_id_master(mptn_id):
            return True

        if self._is_id_gwself(mptn_id):
            return True

        if self._is_id_in_gwself_network(mptn_id):
            return True

        if self._find_nexthop_for_id(mptn_id) is not None:
            return True

        return False

    def clear_id_req_queue(self):
        wait_sec = CONFIG.UNITTEST_WAIT_SEC
        while True:
            message = self._id_req_queue.get()

            if MPTN.socket_send(None, MPTN.MASTER_ID, message, expect_reply=True) is None:
                self._id_req_queue.put_nowait(message)
                gevent.sleep(wait_sec)
            gevent.sleep(wait_sec/2)

    def rtping_forever(self):
        while True:
            message = MPTN.create_packet_to_str(MPTN.MASTER_ID, self._id, MPTN.MPTN_MSGTYPE_RTPING, self._nexthop_hash)
            MPTN.socket_send(None, MPTN.MASTER_ID, message)
            gevent.sleep(5)

    '''
    Handle message functions
    '''
    def handle_idreq_message(self, context, dest_id, src_id, msg_type, payload):
        if not self._is_id_master(dest_id):
            logger.error("IDREQ dest ID should be 0 not %X (%s)" % (dest_id, MPTN.ID_TO_STRING(dest_id)))
            return

        if src_id != MPTN.MPTN_MAX_ID:
            logger.error("IDREQ src ID should be 0xFFFFFFFF not %X (%s)" % (dest_id, MPTN.ID_TO_STRING(dest_id)))
            return

        if payload is None or len(payload) != MPTN.IDREQ_PAYLOAD_LEN:
            logger.error("IDREQ length of payload %d should be %d" % (len(payload), MPTN.IDREQ_PAYLOAD_LEN))
            return

        uuid = payload

        try:
            temp_addr = int(context.address)
        except Exception as e:
            logger.error("IDREQ cannot turn interface address %s into integer" % str(context.address))
            exit(-1)

        if self._transport_if_addr_len == 1 or self._transport_if_addr_len == 2:
            temp_id = self._id_prefix | temp_addr

        elif self._transport_if_addr_len == 4:
            temp_id = temp_addr

        dest_id = self._id
        src_id = temp_id

        # New addresss from transport interface
        if not self.is_addr_valid(temp_addr):
            if CONFIG.UNITTEST_MODE:
                message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, uuid)
                self._id_req_queue.put_nowait(message)

                dest_id = temp_id
                src_id = MPTN.MASTER_ID
                msg_type = MPTN.MPTN_MSGTYPE_IDACK
                message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, uuid)
                self._transport_if_send(temp_addr, message)
                return

            else:
                message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, uuid)
                packet = MPTN.socket_send(context, MPTN.MASTER_ID, message, expect_reply=True)
                if packet is None:
                    logger.error("IDREQ cannot be confirmed ID=%d (%s) Addr=%d" % (temp_id, MPTN.ID_TO_STRING(temp_id), temp_addr))
                    return
                dest_id, src_id, msg_type, payload = packet
                if dest_id != temp_id or src_id != MPTN.MASTER_ID or (msg_type not in [MPTN.MPTN_MSGTYPE_IDACK, MPTN.MPTN_MSGTYPE_IDNAK]):
                    logger.error("IDREQ invalid response for dest ID=%X (%s), src ID=%X (%s), msg_type=%X" % (dest_id, MPTN.ID_TO_STRING(dest_id), src_id, MPTN.ID_TO_STRING(src_id), msg_type) )
                    return

                if msg_type == MPTN.MPTN_MSGTYPE_IDNAK:
                    logger.error("IDREQ for %X (%s) is refused by Master" % (temp_id, MPTN.ID_TO_STRING(temp_id)))
                    return

                self._alloc_address(temp_addr, uuid)
                message = MPTN.create_packet_to_str(dest_id, src_id, MPTN.MPTN_MSGTYPE_IDACK, None)
                self._transport_if_send(temp_addr, message)
                return

        # Known address to check uuid
        elif self._addr_db[temp_addr] == payload:
            dest_id = temp_id
            message = MPTN.create_packet_to_str(dest_id, MPTN.MASTER_ID, MPTN.MPTN_MSGTYPE_IDACK, None)
            self._transport_if_send(temp_addr, message)
            return

        else:
            logger.error("IDREQ comes with a valid addr=%d, ID=%d or %s, but an unknown uuid %s" % (temp_addr, temp_id, MPTN.ID_TO_STRING(temp_id), str(map(ord, uuid))))

    def handle_fwdreq_message(self, context, dest_id, src_id, msg_type, payload):
        if payload is None:
            logger.error("FWDREQ should have the payload")
            return

        if not self.is_id_valid(src_id):
            logger.error("invalid FWDREQ src ID %X %s: not found in network" % (src_id, MPTN.ID_TO_STRING(src_id)))
            return

        if self._is_id_master(dest_id):
            logger.debug("FWDREQ is to master")
            message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, payload)
            if not self._forward_to_next_hop(context, dest_id, message):
                logger.error("FWDREQ to master failed")
            return # no need to return FWDACK back via transport interface

        if self._is_id_gwself(dest_id):
            logger.debug("FWDREQ the message is to me")
            if context.direction == MPTN.ONLY_FROM_TRANSPORT_INTERFACE:
                payload = map(ord, payload)
                handler = self._app_handler.get(payload[0])
                if handler is not None:
                    handler(context.address, payload[1:])
                    gevent.sleep(0.001)
                else:
                    logger.error("FWDREQ receives invalid gateway application %d" % payload[0])
            else:
                logger.error("FWDREQ receives invalid message to me from Master or other gateways")
            return

        if self._is_id_in_gwself_network(dest_id):
            logger.debug("FWDREQ to transport interface directly")
            message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, payload)
            ret = self._transport_if_send(self._get_address_from_id(dest_id), message)

            msg_type = MPTN.MPTN_MSGTYPE_FWDACK
            if not ret[0]:
                msg_type = MPTN.MPTN_MSGTYPE_FWDNAK
                logger.error("FWDREQ to transport address %X fail" % self._get_address_from_id(dest_id))

            if not self._is_id_in_gwself_network(src_id):
                message = MPTN.create_packet_to_str(src_id, dest_id, msg_type, payload)
                MPTN.socket_send(context, context.id, message)

            return

        logger.debug("FWDREQ may be sent to other gateway's network")
        message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, payload)
        if self._forward_to_next_hop(context, dest_id, message):
            return

        logger.error("FWDREQ dest ID %X %s is neither the master, the gateway, nor within MPTN" % (dest_id, MPTN.ID_TO_STRING(dest_id)))
        return

    def handle_gwdiscover_message(self, context, dest_id, src_id, msg_type, payload):
        if context.direction != MPTN.ONLY_FROM_TRANSPORT_INTERFACE:
            logger.error("GWDISCOVER cannot be from TCP Server")
            return

        if payload is not None:
            logger.error("GWDISCOVER should not have the payload")
            return

        if dest_id != MPTN.MPTN_MAX_ID:
            logger.error("GWDISCOVER dest_id should be 0xFFFFFFFF")
            return

        if not MPTN.IS_ID_IN_NETWORK(src_id, self._network):
            logger.error("GWDISCOVER src ID %X %s does not belong to the network" % (src_id, MPTN.ID_TO_STRING(src_id)))
            return

        msg_type = MPTN.MPTN_MSGTYPE_GWOFFER
        message = MPTN.create_packet_to_str(dest_id, self._id, msg_type, uuid.uuid4().bytes)
        self._transport_if_send(self._get_address_from_id(src_id), message)

    def handle_rtping_message(self, context, dest_id, src_id, msg_type, payload):
        if payload is None:
            logger.error("RTPING should have the payload")
            return

        if dest_id != self._id:
            logger.error("RTPING dest_id should be me")
            return

        if not self._is_id_master(src_id):
            logger.error("RTPING src ID %X %s should be Master 0" % (src_id, MPTN.ID_TO_STRING(src_id)))
            return

        if payload != self._nexthop_hash:
            logger.debug("RTPING got different hash %s. mine is %s. need to update routing table" % (str(map(ord, payload)), str(map(ord, self._nexthop_hash))))
            message = MPTN.create_packet_to_str(MPTN.MASTER_ID, self._id, MPTN.MPTN_MSGTYPE_RTREQ, None)
            MPTN.socket_send(None, MPTN.MASTER_ID, message)

    def handle_rtreq_message(self, context, dest_id, src_id, msg_type, payload):
        logger.error("RTREQ does not implement")

    def handle_rtrep_message(self, context, dest_id, src_id, msg_type, payload):
        if payload is None:
            logger.error("RTREP should have the payload")
            return

        if dest_id != self._id:
            logger.error("RTREP dest_id should be me")
            return

        if not self._is_id_master(src_id):
            logger.error("RTPING src ID %X %s should be Master 0" % (src_id, MPTN.ID_TO_STRING(src_id)))
            return

        if payload is None:
            logger.error("RTREP payload should not be empty")
            return

        try:
            rtrep_nexthop = json.loads(payload)
        except Exception as e:
            logger.error("RTREP payload %s cannot be loaded as json. error=%s\n%s" % (payload, str(e), traceback.format_exc()))
            return

        self._nexthop_lookup.clear()
        try:
            for network_string, tcp_address in rtrep_nexthop.iteritems():
                network = MPTN.ID_NETWORK_FROM_STRING(network_string)
                if MPTN.IS_ID_IN_NETWORK(self._id, network): continue
                self._nexthop_lookup[network] = MPTN.NextHop(id=MPTN.ID_FROM_STRING(network_string),tcp_address=tuple(tcp_address))
        except Exception as e:
            logger.error("RTREP got an incorrect json as payload %s. error=%s\n%s" % (payload, str(e), traceback.format_exc()))
            self._nexthop_lookup = {MPTN.ID_NETWORK_FROM_STRING(network_string):MPTN.NextHop(id=MPTN.ID_FROM_STRING(network_string),tcp_address=tcp_address) for (network_string, tcp_address) in self._nexthop_db.iteritems()}
            return

        self._nexthop_db.clear()
        for network_string, tcp_address in rtrep_nexthop.iteritems():
            self._nexthop_db[network_string] = tuple(tcp_address)

        self._update_nexthop_hash()
        # logger.info("new hash is %s" % str(map(ord, payload)))
        # logger.info("db %s lookup %s" % (str(self._nexthop_db), str(self._nexthop_lookup)))
        return
