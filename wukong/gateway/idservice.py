try:
    import netifaces
except:
    print "Please install the netifaces module from pypi"
    print "e.g. sudo pip install netifaces"
    exit(-1)

try:
    import pytrie
except:
    print "Please install the netifaces module from pypi"
    print "e.g. sudo pip install pytrie"
    exit(-1)

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
import color_logging, logging
logger = logging

RANDOM_BYTES = map(ord, uuid.uuid4().bytes)

class IDService(object):
    def __init__(self, transport_if_addr, transport_if_len, autonet_mac_addr=[], gateway_application_handlers={}):
        self._app_handler = gateway_application_handlers
        self._transport_if_addr = transport_if_addr
        self._transport_if_addr_len = transport_if_len

        # _addr_db: key = address, value = True or False
        self._addr_db = DBDict("gtw_addr_table.sqlite")

        # _nexthop_db: key = "MPTN ID/NETMASK" STRING, value = next hop's tcp_address tuple ("IP" STRING, PORT INT)
        self._nexthop_db = DBDict("gtw_nexthop_table.sqlite")
        self._init_nexthop_lookup()

        # _value_db: key = MPTN ID, value = unique value (such as MAC address)
        self._value_db = DBDict("gtw_value_table.sqlite")

        self._settings_db = DBDict("gtw_settings_db.sqlite")
        self._init_settings_db(autonet_mac_addr)

        if CONFIG.UNITTEST_MODE:
            self._id_req_queue = Queue()

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

            # try:
            #     nc_if = netifaces.ifaddresses(self._transport_if_addr[0])[netifaces.AF_INET][0]
            #     interface = MPTN.ID_INTERFACE_FROM_TUPLE( net_if['addr'], net_if['netmask'])
            # except (IndexError, KeyError, ValueError):
            #     logger.error("cannot find any IP address from the the network interface %s" % CONFIG.TRANSPORT_INTERFACE_ADDR)
            #     self._clear_settings_db()
            #     exit(-1)

            interface = MPTN.ID_INTERFACE_FROM_TUPLE(self._transport_if_addr[0], self._transport_if_addr[1])

        else:
            logger.error("_init_settings_db Unsupported interface type: %s" % CONFIG.TRANSPORT_INTERFACE_TYPE)
            self._clear_settings_db()
            exit(-1)

        self._transport_if_addr = int(interface.ip)
        self._transport_network_size = interface.network.num_addresses
        self._id_hostmask = int(interface.network.hostmask)
        self._id_prefix_len = interface.network.prefixlen
        self._id_netmask = int(interface.network.netmask)

        if "GTWSELF_UNIQUE_VALUE" not in self._settings_db:
            if len(autonet_mac_addr) < MPTN.GWIDREQ_PAYLOAD_LEN:
                autonet_mac_addr = RANDOM_BYTES[0:MPTN.GWIDREQ_PAYLOAD_LEN-len(autonet_mac_addr)] + autonet_mac_addr
            self._settings_db["GTWSELF_UNIQUE_VALUE"] = autonet_mac_addr
            logger.debug("GTWSELF_UNIQUE_VALUE is %s" % str(autonet_mac_addr))

        # Check whether master is online and gateway's own "MPTN ID/prefix" is valid or not
        self._init_get_prefix_from_master()
        if self._id == 0xFFFFFFFF:
            logger.error("_init_settings_db cannot initialize gateway because of no valid ID")
            self._clear_settings_db()
            exit(-1)

    def _clear_settings_db(self):
        for d in [self._settings_db, self._addr_db, self._nexthop_db, self._value_db]:
            d.clear()

    def _init_get_prefix_from_master(self):
        if "GTWSELF_ID" not in self._settings_db: self._settings_db["GTWSELF_ID"] = 0xFFFFFFFF
        self._id = self._settings_db["GTWSELF_ID"]
        MPTN.set_self_id(self._id)
        if self._id != 0xFFFFFFFF:
            self._network = MPTN.ID_NETWORK_FROM_TUPLE(MPTN.ID_TO_STRING(self._id),str(self._id_prefix_len))
            self._id_prefix = self._id & self._id_netmask

        dest_id = MPTN.MASTER_ID
        src_id = self._id
        msg_type = MPTN.MPTN_MSGTYPE_GWIDREQ

        payload = json.dumps({"IFADDR":self._transport_if_addr,
            "IFADDRLEN":self._transport_if_addr_len,
            "IFNETMASK":self._id_netmask, "PORT":CONFIG.SELF_TCP_SERVER_PORT,
            "VAL":self._settings_db["GTWSELF_UNIQUE_VALUE"]})
        # payload = "IFADDR=%d;IFADDRLEN=%d;IFNETMASK=%d;PORT=%d;VAL=%s" % (self._transport_if_addr,
        #     self._transport_if_addr_len, self._id_netmask, CONFIG.SELF_TCP_SERVER_PORT,
        #     struct.pack("!%dB"%MPTN.GWIDREQ_PAYLOAD_LEN, *self._settings_db["GTWSELF_UNIQUE_VALUE"]))
        message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, payload)

        packet = MPTN.socket_send(None, MPTN.MASTER_ID, message, expect_reply=True)
        if packet is None:
            logger.error("GWIDREQ cannot get GWIDACK/NAK from master due to network problem")
            return None

        # log_msg = MPTN.formatted_print(MPTN.split_packet_to_list(message))
        dest_id, src_id, msg_type, _ = packet

        if msg_type == MPTN.MPTN_MSGTYPE_GWIDNAK:
            logger.error("GWIDREQ GWIDREQ is refused by master")
            return None

        elif msg_type != MPTN.MPTN_MSGTYPE_GWIDACK:
            logger.error("GWIDREQ get unexpected msg type (%d) instead of GWIDACK/NAK from master" % msg_type)
            return None

        elif dest_id != self._id:
            if self._id == 0xFFFFFFFF:
                self._settings_db["GTWSELF_ID"] = dest_id
                self._id = dest_id
                MPTN.set_self_id(self._id)
                self._network = MPTN.ID_NETWORK_FROM_TUPLE(MPTN.ID_TO_STRING(self._id),str(self._id_prefix_len))
                self._id_prefix = dest_id & self._id_netmask
                logger.info("GWIDREQ successfully get new ID %d including prefix" % dest_id)
            else:
                logger.error("GWIDREQ get an ID %d %s different from old one %d %s" % (dest_id, MPTN.ID_TO_STRING(dest_id), self._id, MPTN.ID_TO_STRING(self._id)))
                exit(-1)
        else:
            logger.info("GWIDREQ successfully check the ID with master")

    def _init_nexthop_lookup(self):
        self._nexthop_lookup = {MPTN.ID_NETWORK_FROM_STRING(network_string):MPTN.NextHop(id=MPTN.ID_FROM_STRING(network_string),tcp_address=tcp_address) for (network_string, tcp_address) in self._nexthop_db.iteritems()}
        self._nexthop_hash = hash(frozenset(self._nexthop_lookup.items()))
        MPTN.set_nexthop_lookup_function(self._find_nexthop_for_id)

    def _alloc_address(self, address):
        assert isinstance(address, (int, long)), "_alloc_address %s must be integer instead of %s" % (str(address), type(address))
        assert address < self._transport_network_size, "_alloc_address %d cannot excede upper bound %d" % (address, self._transport_network_size)
        self._addr_db[address] = True

    def _dealloc_address(self, address):
        if self.is_address_valid(address):
            del self._addr_db[address]

    def _get_address_from_id(self, mptn_id):
        return mptn_id & self._id_hostmask

    def _is_id_master(self, mptn_id):
        return mptn_id == MPTN.MASTER_ID

    def _is_id_gwself(self, mptn_id):
        return mptn_id == self._id

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
            logger.error("_forward_to_next_hop cannot forward packet to %s due to network problem" % str(address))
            return False

        # log_msg = MPTN.formatted_print(MPTN.split_packet_to_list(packet))
        dest_id, src_id, msg_type, payload = packet

        if payload is None:
            logger.error("_forward_to_next_hop FWDACK/NAK from master might not have the payload")
            return False

        if msg_type == MPTN.MPTN_MSGTYPE_FWDNAK:
            logger.error("_forward_to_next_hop forward via %s fails with error %s" % (str(address), payload))
            return False

        if msg_type != MPTN.MPTN_MSGTYPE_FWDACK:
            logger.error("_forward_to_next_hop get unexpected msg type (%d) instead of FWDACK/NAK" % msg_type)
            return False

        # TODO: may need retransmittion? Could be redundant since WKPF has it.
        return True

    '''
    Public functions
    '''
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

    def rt_ping_forever(self):
        while True:
            gevent.sleep(5)

    '''
    Handle message functions
    '''
    def handle_idreq_message(self, context, dest_id, src_id, msg_type, payload):
        if not self._is_id_master(dest_id):
            logger.error("IDREQ dest ID should be 0 not %X (%s)" % (dest_id, MPTN.ID_TO_STRING(dest_id)))
            return

        if src_id != 0xFFFFFFFF:
            logger.error("IDREQ src ID should be 0xFFFFFFFF not %X (%s)" % (dest_id, MPTN.ID_TO_STRING(dest_id)))
            return

        if payload is None or len(payload) != MPTN.IDREQ_PAYLOAD_LEN:
            logger.error("IDREQ length of payload %d should be %d" % (len(payload), MPTN.IDREQ_PAYLOAD_LEN))
            return

        value = payload

        try:
            temp_addr = int(context.address)
        except Exception as e:
            logger.error("IDREQ cannot turn interface address %s into integer" % str(context.address))
            exit(-1)
        temp_id = self._id_prefix | temp_addr

        dest_id = self._id
        src_id = temp_id

        # New addresss from transport interface
        if not self.is_addr_valid(temp_addr):
            if CONFIG.UNITTEST_MODE:
                message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, value)
                self._id_req_queue.put_nowait(message)

                dest_id = temp_id
                src_id = MPTN.MASTER_ID
                msg_type = MPTN.MPTN_MSGTYPE_IDACK
                message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, value)
                MPTN.transport_if_send(temp_addr, message)
                return

            else:
                message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, value)
                packet = MPTN.socket_send(context, MPTN.MASTER_ID, message, expect_reply=True)
                if packet is None:
                    logger.error("IDREQ cannot be confirmed ID=%d (%s) Addr=%d (%s)" % (temp_id, MPTN.ID_TO_STRING(temp_id), temp_addr))
                    return
                dest_id, src_id, msg_type, _ = packet
                if dest_id != temp_id or src_id != MPTN.MASTER_ID or msg_type != MPTN.MPTN_MSGTYPE_IDACK:
                    logger.error("IDREQ invalid responce for dest ID=%X (%s), src ID=%X (%s)" % (dest_id, MPTN.ID_TO_STRING(dest_id), src_id, MPTN.ID_TO_STRING(src_id)) )
                    return
                self._alloc_address(temp_addr)
                self._value_db[temp_id] = value
                return

        # Known address to check value
        elif self._value_db[temp_id] == payload:
            message = MPTN.create_packet_to_str(src_id, dest_id, MPTN.MPTN_MSGTYPE_IDACK, None)
            MPTN.transport_if_send(temp_addr, message)
            return

        else:
            logger.error("IDREQ comes with a valid ID %d %s and an unknown value %s" % (temp_id, MPTN.ID_TO_STRING(temp_id), str(map(ord, value))))

    def handle_fwdreq_message(self, context, dest_id, src_id, msg_type, payload):
        if payload is None:
            logger.error("FWDREQ should have the payload")
            return

        if not self.is_id_valid(src_id):
            logger.error("invalid FWDREQ src ID %X %s: not found in network" % (src_id, MPTN.ID_TO_STRING(src_id)))
            return

        if self._is_id_master(dest_id):
            logger.debug("FWDREQ is to master")
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
            message = map(ord, MPTN.create_packet_to_str(dest_id, src_id, msg_type, payload))
            ret = self._transport_if_send(self._get_address_from_id(dest_id), message)

            msg_type = MPTN.MPTN_MSGTYPE_FWDACK
            if not ret[0]:
                msg_type = MPTN.MPTN_MSGTYPE_FWDACK
                logger.error("FWDREQ to transport address %X fail" % self._get_address_from_id(dest_id))

            message = MPTN.create_packet_to_str(src_id, dest_id, msg_type, None)
            MPTN.socket_send(context, src_id, message)
            return

        logger.debug("FWDREQ may be sent to other gateway's network")
        if self._forward_to_next_hop(context, next_hop.id, message):
            return

        logger.error("FWDREQ dest ID %X %s is neither the master, the gateway, nor within MPTN" % (dest_id, MPTN.ID_TO_STRING(dest_id)))
        return

    def handle_rtping_message(self, context, dest_id, src_id, msg_type, payload):
        raise NotImplementedError

    def handle_rtreq_message(self, context, dest_id, src_id, msg_type, payload):
        raise NotImplementedError