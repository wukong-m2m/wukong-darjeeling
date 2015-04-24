try:
    import netifaces
except:
    print "Please install the netifaces module from pypi"
    print "e.g. sudo pip install netifaces"
    exit(-1)

import gtwconfig as CONFIG
import mptnUtils as MPTN
import ipaddress
import dbdict

import gevent
from gevent import socket
from gevent.queue import Queue
import struct
from datetime import datetime
import logging
import random

logging.basicConfig(level=CONFIG.LOG_LEVEL)
logger = logging.getLogger( __name__ )

AUTONET_MAC_ADDR_LEN = CONFIG.AUTONET_MAC_ADDR_LEN
RANDOM_MAC_ADDR = [0]*8 + [random.randrange(0, 256) for _ in range(0, AUTONET_MAC_ADDR_LEN)]

class IDService(object):
    def __init__(self, transport_if, autonet_mac_addr=None):

        # _addr_info: key = address, value = True or False
        self._addr_info = dbdict.DBDict("gtw_addr_table.sqlite")
        # _network_info: key = "MPTN ID/MASK", value = "IP:PORT"
        self._network_info = dbdict.DBDict("gtw_network_table.sqlite")
        # _node_info: key = MPTN ID, value = unique value (such as MAC address)
        self._node_info = dbdict.DBDict("gtw_node_table.sqlite")

        self._settings = dbdict.DBDict("gtw_settings.sqlite")
        self._init_settings(transport_if, autonet_mac_addr)

        # Check whether master is online and gateway's own "MPTN ID/prefix" is valid or not
        self._init_get_prefix_from_master()
        if self._id is None:
            logger.error("cannot initialize gateway because of no prefix found")
            exit(-1)

        # Check routing table
        self._init_rtping()

        logger.info("gateway ID is %s (=%d)" % (str(ipaddress.ip_address(self._settings["GTWSELF_ID"])),self._settings["GTWSELF_ID"]))

        if CONFIG.UNITTEST_MODE:
            self._id_req_queue = Queue()
            self._wait_sec = CONFIG.UNITTEST_WAIT_SEC

        logger.info("initialized")

    def _init_settings(self, transport_if, autonet_mac_addr=None):
        self._settings["GTWSELF_UNIQUE_VALUE"] = autonet_mac_addr if autonet_mac_addr is not None else RANDOM_MAC_ADDR
        self._transport_if_addr = transport_if.get_address()
        self._transport_if_addr_len = transport_if.get_addr_len()

        if self._transport_if_addr_len > MPTN.MPTN_LEN_DID:
            logger.error("the length of address (%d) is too long to support" % transport_if_addr_len)
            exit(-1)

        if (CONFIG.TRANSPORT_INTERFACE_TYPE == "zwave" or CONFIG.TRANSPORT_INTERFACE_TYPE == "zigbee"):
            self._transport_network_size = 2 ** (self._transport_if_addr_len*8)

            if isinstance(self._transport_if_addr, int) and self._transport_if_addr >= self._transport_network_size:
                logger.error("transport interface address (%d) is not in the network of length %d" % (transport_if_addr, transport_if_addr_len))
                exit(-1)

            self._id_hostmask = self._transport_network_size-1
            self._id_prefix_len = (MPTN.MPTN_LEN_DID - self._transport_if_addr_len)*8
            self._id_netmask = ((2 ** self._id_prefix_len) - 1) << (self._transport_if_addr_len*8)

        elif CONFIG.TRANSPORT_INTERFACE_TYPE == 'udp':
            try:
                nc_if = netifaces.ifaddresses(CONFIG.TRANSPORT_INTERFACE_ADDR)[netifaces.AF_INET][0]
                ip_if = ipaddress.ip_interface("%s/%s" % ( net_if['addr'], net_if['netmask']))
            except (IndexError, KeyError, ValueError):
                logger.error("cannot find any IP address from the the network interface %s" % CONFIG.TRANSPORT_INTERFACE_ADDR)
                self._clear_settings()
                exit(-1)

            self._transport_if_addr = int(ip_if.ip)
            self._transport_network_size = ip_if.network.num_addresses
            self._id_hostmask = int(ip_if.network.hostmask)
            self._id_prefix_len = ip_if.network.prefixlen * 8
            self._id_netmask = int(ip_if.network.netmask)

    def _clear_settings(self):
        for d in [self._settings, self._addr_info, self._network_info, self._node_info]:
            for key in d:
                del d[key]

    def _init_get_prefix_from_master(self):
        if "GTWSELF_ID" not in self._settings: self._settings["GTWSELF_ID"] = None

        self._id = self._settings["GTWSELF_ID"]
        if self._id is not None:
            self._network = ipaddress.ip_interface("%s/%d"%(str(ipaddress.ip_address(self._id)),self._id_prefix_len)).network
            self._id_prefix = self._id & self._id_netmask

        dest_id = MPTN.MASTER_ID
        src_id = self._id if self._id is not None else 0xFFFFFFFF
        msg_type = MPTN.MPTN_MSGTYPE_GWIDREQ
        payload = "ADDR=%d;LEN=%d;PORT=%d;VAL=%s" % (self._transport_if_addr, self._transport_if_addr_len, CONFIG.SELF_TCP_SERVER_PORT, struct.pack("!16B",*self._settings["GTWSELF_UNIQUE_VALUE"]))
        message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, payload)

        packet = MPTN.send_and_wait_for_reply(MPTN.MASTER_ID, CONFIG.MASTER_ADDRESS, message)
        if packet is None:
            logger.error("cannot get GWIDACK/NAK from master due to network problem")
            return None

        # log_msg = MPTN.formatted_print(MPTN.split_packet_header(packet))
        dest_id, src_id, msg_type, _ = packet

        if msg_type == MPTN.MPTN_MSGTYPE_GWIDNAK:
            logger.error("GWIDREQ is refused by master")
            return None

        elif msg_type != MPTN.MPTN_MSGTYPE_GWIDACK:
            logger.error("get unexpected msg type (%d) instead of GWIDACK/NAK from master" % msg_type)
            return None

        elif dest_id != self._id:
            if self._id is None:
                self._settings["GTWSELF_ID"] = dest_id
                self._id = dest_id
                self._network = ipaddress.ip_interface("%s/%d"%(str(ipaddress.ip_address(dest_id)),self._id_prefix_len)).network
                self._id_prefix = dest_id & self._id_netmask
                logger.info("successfully get new ID %d including prefix" % dest_id)
            else:
                logger.error("get an ID %d different from old one %d" % (dest_id, self._id))
                exit(-1)
        else:
            logger.info("successfully check the ID with master")

    def _init_rtping(self):
        pass

    def _alloc_address(self, address, value=True):
        assert isinstance(address, (int, long)), "address(%s) must be integer instead of %s" % (str(address), type(address))
        assert address < self._transport_network_size, "address(%d) cannot excede upper bound %d" % (address, self._transport_network_size)
        self._addr_info[address] = value

    def _dealloc_address(self, address):
        if self.is_address_valid(address):
            del self._addr_info[address]

    def _get_address_from_id(self, mptn_id):
        return mptn_id & self._id_hostmask

    def _is_id_gwself(self, mptn_id):
        return mptn_id == self._id

    def _is_id_in_gwself_network(self, mptn_id):
        if ipaddress.ip_address(mptn_id) in self._network:
            return self.is_addr_valid(self._get_address_from_id(mptn_id))
        return False

    def _find_network_for_id(self, mptn_id):
        ip_addr = ipaddress.ip_address(mptn_id)
        # _network_info: key = "MPTN ID/MASK", value = "IP:PORT"
        for net_if in self._network_info:
            if ip_addr in ipaddress.ip_interface(net_if).network:
                return self._network_info[net_if]
            gevent.sleep(0)
        return None

    def _forward_to_next_hop(self, address, message):
        # assert isinstance(address, tuple) and len(address) == 2, "address should be a tuple"
        packet = MPTN.send_and_wait_for_reply(dest_id, address, message)

        if packet is None:
            logger.error("cannot forward packet to %s due to network problem" % str(address))
            return None

        # log_msg = MPTN.formatted_print(MPTN.split_packet_header(packet))
        dest_id, src_id, msg_type, payload = packet

        if payload is None:
            logger.error("packet FWD ACK/NAK from master might not have the payload")
            return None

        if msg_type == MPTN.MPTN_MSGTYPE_FWDNAK:
            logger.error("forward via %s fails with error %s" % (str(address), payload))
            return None

        if msg_type != MPTN.MPTN_MSGTYPE_FWDACK:
            logger.error("get unexpected msg type (%d) instead of FWDACK/NAK" % msg_type)
            return None

        # TODO: may need retransmittion?
        return None

    '''
    Public functions
    '''
    def is_id_master(self, mptn_id):
        return mptn_id == MPTN.MASTER_ID

    def is_addr_valid(self, addr):
        return addr in self._addr_info

    def is_id_valid(self, mptn_id):
        if self.is_id_master(mptn_id):
            return True

        if self._is_id_gwself(mptn_id):
            return True

        if self._is_id_in_gwself_network(mptn_id):
            return True

        if self._find_network_for_id(mptn_id) is not None:
            return True

        return False

    def clear_id_req_queue(self):
        wait_sec = CONFIG.UNITTEST_WAIT_SEC
        while True:
            tmp_id, message = self._id_req_queue.get()
            if MPTN.send_and_wait_for_reply(MPTN.MASTER_ID, CONFIG.MASTER_ADDRESS, message) is None:
                self._id_req_queue.put_nowait(message)
                gevent.sleep(wait_sec)
            gevent.sleep(0.05)

    def rt_ping_forever(self):
        while True:
            gevent.sleep(10)

    '''
    Handle message functions
    '''
    def handle_id_req_message(self, context, dest_id, src_id, msg_type, payload):
        if dest_id != 0:
            logger.error("IDREQ dest ID should be 0 not %X" % dest_did)
            return None

        if src_id != 0xFFFFFFFF:
            logger.error("IDREQ src ID should be 0xFFFFFFFF not %X" % dest_did)
            return None

        if payload is None or len(payload) != AUTONET_MAC_ADDR_LEN:
            logger.error("the length of payload of IDREQ %d should be %d" % (len(payload), AUTONET_MAC_ADDR_LEN))
            return None

        temp_radio_addr = context
        temp_id = self._id_prefix | temp_radio_addr

        dest_id = self._id
        src_id = temp_id
        message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, payload)

        # Unregistered addresss from transport interface
        if not self.is_addr_valid(temp_radio_addr):
            if CONFIG.UNITTEST_MODE:
                self._id_req_queue.put_nowait(message)
            else:
                packet = MPTN.send_and_wait_for_reply(MPTN.MASTER_ID, CONFIG.MASTER_ADDRESS, message)

        _, _, msg_type, _ = packet
        if msg_type == MPTN.MPTN_MSGTYPE_IDACK or CONFIG.UNITTEST_MODE:
            dest_id = temp_id
            src_id = MPTN.MASTER_ID
            message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, None)
            return header
        else:
            return None

    def handle_fwd_req_message(self, context, dest_id, src_id, msg_type, payload):
        if payload is None:
            logger.error("packet FWDREQ should have the payload")
            return None

        if self.is_id_valid(src_id):
            logger.error("invalid FWDREQ src ID %X: not found in network" % src_id)

        if self.is_id_master(dest_id):
            logger.debug("forward the message to master")
            self._forward_to_next_hop(CONFIG.MASTER_ADDRESS, message)
            return None

        if self._is_id_gwself(dest_id):
            logger.debug("the message is mine")
            return self._get_address_from_id(dest_id)

        if self._is_id_in_gwself_network(dest_id):
            logger.debug("forward the message to transport interface directly")
            return self._get_address_from_id(dest_id)

        net = self._find_network_for_id(dest_id) # got next hop's "IP:PORT"
        if net is not None:
            address = net.split(":")
            logger.debug("forward the message to other known gateway")
            self._forward_to_next_hop((address[0], int(address[1])), message)
            return None
        else:
            logger.error("the FWDREQ dest ID %X is neither the master, the gateway, nor within all networks" % dest_id)
            return None

    def handle_rt_ping_message(self, context, dest_id, src_id, msg_type, payload):
        pass

    def handle_rt_req_message(self, context, dest_id, src_id, msg_type, payload):
        pass