import binascii
import struct
import os
import gevent
import ipaddress
import time
from gevent.lock import RLock
from gevent.event import AsyncResult
from gevent import socket
import collections
import traceback

try:
    import color_logging
    import logging
    logger = logging
except:
    import logging
    logger = logging
    logger.debug = logger.warning

ProtocolHandler = collections.namedtuple("ProtocolHandler", "permission, handler")
NonceCallback = collections.namedtuple("NonceCallback", "id, callback")
NextHop = collections.namedtuple("NextHop", "id, tcp_address")
Peer = collections.namedtuple("Peer", "socket, id, address")

CONNECTION_RETRIES = 1
NETWORK_TIMEOUT = 3.0

ZW_ADDRESS_LEN = 1
ZB_ADDRESS_LEN = 2
IP_ADDRESS_LEN = 4

STOP_MODE = (0, "STOP")
ADD_MODE = (1, "ADD")
DEL_MODE = (2, "DELTE")

ONLY_FROM_TCP_SERVER = 1
ONLY_FROM_TRANSPORT_INTERFACE = 2
VALID_FROM_ALL = 3

MASTER_ID = 0
MPTN_UDP_PORT = 5775 # 0x57 for 'W', 0x75 for 'u'

MPTN_TCP_PACKET_SIZE = struct.calcsize("!L")
MPTN_TCP_NONCE_SIZE = struct.calcsize("!8B")

GWIDREQ_PAYLOAD_LEN = 16
IDREQ_PAYLOAD_LEN = 16

ID_TO_STRING = lambda x: str(ipaddress.ip_address(x))
ID_INTERFACE_FROM_TUPLE = lambda ip, mask: ipaddress.ip_interface("%s/%s" % (ip, mask))
ID_INTERFACE_FROM_STRING = lambda x: ipaddress.ip_interface(x)
ID_NETWORK_FROM_TUPLE = lambda ip, mask: ipaddress.ip_interface("%s/%s" % (ip, mask)).network
ID_NETWORK_FROM_STRING = lambda x: ipaddress.ip_interface(x).network
ID_FROM_STRING = lambda x: int(ipaddress.ip_interface(x).ip)
IS_ID_IN_NETWORK = lambda x, network: ipaddress.ip_address(x) in network
VALUE_TO_STRING = lambda value: ":".join(map(lambda x:"%02X"%x, value))

# Acceptable payload format
# 4 bytes: destionation DID
# 4 bytes: source DID
# 1 byte: message type
# rest byte(s): payload
MPTN_ID_LEN                = 4
MPTN_MAX_ID                = 2 ** (MPTN_ID_LEN*8) - 1
MPTN_MSGTYPE_LEN            = 1

MPTN_HEADER_FORMAT          = (MPTN_ID_LEN, MPTN_ID_LEN, MPTN_MSGTYPE_LEN)
MPTN_DEST_BYTE_OFFSET       = 0
MPTN_SRC_BYTE_OFFSET        = MPTN_DEST_BYTE_OFFSET + MPTN_ID_LEN
MPTN_MSGTYPE_BYTE_OFFSET    = MPTN_SRC_BYTE_OFFSET + MPTN_ID_LEN
MPTN_PAYLOAD_BYTE_OFFSET    = MPTN_MSGTYPE_BYTE_OFFSET + MPTN_MSGTYPE_LEN

MPTN_MSGTYPE_GWDISCOVER     = 0
MPTN_MSGTYPE_GWOFFER        = 1
MPTN_MSGTYPE_IDREQ          = 2
MPTN_MSGTYPE_IDACK          = 3
MPTN_MSGTYPE_IDNAK          = 4
MPTN_MSGTYPE_GWIDREQ        = 5
MPTN_MSGTYPE_GWIDACK        = 6
MPTN_MSGTYPE_GWIDNAK        = 7

MPTN_MSGTYPE_RTPING         = 8
MPTN_MSGTYPE_RTREQ          = 9
MPTN_MSGTYPE_RTREP          = 10

MPTN_MSGTYPE_RPCCMD         = 16
MPTN_MSGTYPE_RPCREP         = 17

MPTN_MSGTYPE_FWDREQ         = 24
MPTN_MSGTYPE_FWDACK         = 25
MPTN_MSGTYPE_FWDNAK         = 26

WKPF_COMMAND_MONITOR        = 0xB5

HEADER_FORMAT_STR = "!" + ''.join([{1:'B',2:'H',4:'I',8:'Q'}[i] for i in MPTN_HEADER_FORMAT])

def split_packet_to_list(message):
    ret = []
    for i in MPTN_HEADER_FORMAT:
        ret.append(message[:i])
        message = message[i:]
    if len(message) > 0:
        ret.append(message)
    return ret

def formatted_print(msg):
    """Receives all message parts from socket, printing each frame neatly"""
    r = "----------------------------------------\n"
    for part in msg:
        r += "[%03d]" % len(part) # Trailing comma suppresses newline
        try:
            r += "%s" % part.decode('ascii')
            r += "\t("
            r += r"0x%s" % (binascii.hexlify(part).decode('ascii'))
            r += ")"
        except UnicodeDecodeError:
            r += r"0x%s" % (binascii.hexlify(part).decode('ascii'))
        r += '\n'
    return r

def create_packet_to_str(dest_id, src_id, msg_type, payload):
    header = struct.pack(HEADER_FORMAT_STR, dest_id, src_id, msg_type)
    if payload is not None: return header + payload
    return header

def extract_packet_from_str(s):
    if len(s) < MPTN_PAYLOAD_BYTE_OFFSET: return (None,None,None,None)
    header, payload = s[:MPTN_PAYLOAD_BYTE_OFFSET], s[MPTN_PAYLOAD_BYTE_OFFSET:]
    if payload == "": payload = None
    return struct.unpack(HEADER_FORMAT_STR, header)+(payload,)

class Context(object):
    __slots__ = ("id", "address", "direction", "socket", "nonce")
    def __init__(self, mptn_id, address, direction, sock, nonce):
        self.id = mptn_id
        self.address = address
        self.direction = direction
        self.socket = sock
        self.nonce = nonce

def new_if_context(addr):
    return Context(None, addr, ONLY_FROM_TRANSPORT_INTERFACE, None, None)

process_message_handler = None
def set_message_handler(handler):
    global process_message_handler
    process_message_handler = handler

find_nexthop_for_id = None
def set_nexthop_lookup_function(lookup):
    global find_nexthop_for_id
    find_nexthop_for_id = lookup

self_id_net_endian_string = None
def set_self_id(mptn_id):
    global self_id_net_endian_string
    try:
        self_id_net_endian_string = struct.pack("!L",socket.htonl(mptn_id))
    except Exception as e:
        logger.error("set_self_id unknown error: %s\n%s" % (str(e), traceback.format_exc()))

addr_to_bool_db = None
def set_address_allocation_table(db):
    global addr_to_bool_db
    addr_to_bool_db = db

def get_all_addresses():
    global addr_to_bool_db
    if addr_to_bool_db is None: return None
    # remember that if addr_to_bool_db changes, return values won't reflect that
    return map(int, addr_to_bool_db.keys())

class ConnectionManager(object):
    _manager = None
    @classmethod
    def init(cls):
        if not cls._manager:
            cls._manager = ConnectionManager()
        return cls._manager

    def __init__(self):
        self.mptn_lock = RLock()
        self.mptn_conn_by_addr = {} # map ID to a Peer (address, socket)
        self.mptn_conn_by_id = collections.defaultdict(list)
        self.nonce_lock = RLock()
        self.nonce_cache = {} # map nonce to a NonceCallback (ID, listener) or map ID to a list of nonces [nonce, ...]

    def __repr__(self):
        return "ConnectionManager:\n\tconnections:\n\t\t%s\n\tnonces:\n\t\t%s" % (
            str(self.mptn_conn_by_addr), str(self.nonce_cache))

    def add_peer(self, address, peer):
        with self.mptn_lock:
            logger.debug("add_peer add address %s peer %s" % (str(address), str(peer)))
            self.remove_peer(address)

            self.mptn_conn_by_addr[address] = peer
            self.mptn_conn_by_id[peer.id].append(peer)

    def remove_peer(self, address):
        with self.mptn_lock:
            if address is None or address not in self.mptn_conn_by_addr:
                # logger.info("remove_peer not find address %s. Continue, anyway" % str(address))
                return

            try:
                peer = self.mptn_conn_by_addr.pop(address)
                self.mptn_conn_by_id[peer.id] = [p for p in self.mptn_conn_by_id.pop(peer.id) if p.address != peer.address]
                peer.socket.close()
            except Exception as e:
                logger.error("remove_peer socket closed for ID=%s address %s occurs error=%s\n%s" % (ID_TO_STRING(peer.id), str(address), str(e), traceback.format_exc()))

            logger.info("remove_peer done for ID=%s address %s" % (ID_TO_STRING(peer.id), str(address)))

    def get_peer_by_id(self, mptn_id):
        with self.mptn_lock:
            p_list = self.mptn_conn_by_id.get(mptn_id)
            if p_list is None or len(p_list) == 0: return None
            return p_list[0].socket

    def add_nonce(self, nonce, nncb):
        with self.nonce_lock:
            old_nncb = self.pop_nonce(nonce)
            if old_nncb is not None:
                logger.error("nonce collision occurs. nonce of ID %s's replaces that of old ID %s's" % (ID_TO_STRING(nncb.id), ID_TO_STRING(old_nncb.id)))
                old_nncb.callback.set(None)
                del old_nncb
            self.nonce_cache[nonce] = (nncb, int(round(time.time()*1000))+10000)
            self.remove_timeout_nonce()

    def pop_nonce(self, nonce):
        with self.nonce_lock:
            if nonce not in self.nonce_cache: return None
            self.remove_timeout_nonce()
            return self.nonce_cache.pop(nonce)[0]

    def remove_timeout_nonce(self):
        with self.nonce_lock:
            for nonce, t in self.nonce_cache.items():
                if t[1] < int(round(time.time() * 1000)):
                    t[0].callback.set(None)
                    self.nonce_cache.pop(nonce)


def handle_reply_message(context, dest_id, src_id, msg_type, payload):
    nncb = ConnectionManager.init().pop_nonce(context.nonce)

    if nncb is None:
        logger.error("handle_reply_message no nonce %s exists" % str(map(ord, context.nonce)))
        return

    if nncb.id != src_id:
        logger.error("handle_reply_message ID %s 0x%X not match nonce %s callback ID %s 0x%X" % (ID_TO_STRING(src_id), src_id, str(map(ord, context.nonce)), ID_TO_STRING(nncb.id), nncb.id))
        return
    context.id = src_id

    nncb.callback.set((dest_id, src_id, msg_type, payload))


def handle_socket(sock, addr):
    # logger.debug("handle_socket serve sock %s and addr %s" % (str(sock), str(addr)))
    peer_id_string = ""
    peer_id = MPTN_MAX_ID
    try:
        peer_id_string = sock.recv(MPTN_ID_LEN)
        if peer_id_string == "":
            sock.close()
        peer_id = socket.ntohl(struct.unpack("!L", peer_id_string)[0])
        if peer_id != MPTN_MAX_ID:
            ConnectionManager.init().add_peer(addr, Peer(socket=sock, id=peer_id, address=addr))
    except Exception as e:
        logger.error("handle_socket peer_id_string:%s peer_id:%s error=%s\n%s" % (str(e), peer_id_string, str(ID_TO_STRING(peer_id)), traceback.format_exc()))
    socket_recv(sock, addr, peer_id)

def socket_recv(sock, addr, peer_id):
    while True:
        nonce = ""
        size_string = ""
        size = 0
        message = ""
        try:
            nonce = sock.recv(MPTN_TCP_NONCE_SIZE)
            if nonce == "":
                logger.error("handle_socket closed. nonce. addr=%s" % (str(addr)))
                logger.debug("handle_socket closed. connections before %s" % str(ConnectionManager.init()))
                ConnectionManager.init().remove_peer(addr)
                logger.debug("handle_socket closed. connections after %s" % str(ConnectionManager.init()))
                return

            size_string = sock.recv(MPTN_TCP_PACKET_SIZE)
            if size_string == "":
                logger.error("handle_socket closed. size. addr=%s, nonce=%s" % (str(addr), str(map(ord, nonce))))
                logger.debug("handle_socket closed. connections before %s" % str(ConnectionManager.init()))
                ConnectionManager.init().remove_peer(addr)
                logger.debug("handle_socket closed. connections after %s" % str(ConnectionManager.init()))
                return
            size = socket.ntohl(struct.unpack("!L", size_string)[0])

            while len(message) < size:
                part_msg = sock.recv(size - len(message))
                if part_msg == "":
                    logger.error("handle_socket closed. message. addr=%s, nonce=%s, size_string=%s, size=%d, message=\n%s" % (
                        str(addr), str(map(ord, nonce)), str(map(ord, size_string)), size,
                        str(formatted_print(split_packet_to_list(message)))
                        )
                    )
                    logger.debug("handle_socket closed. connections before %s" % str(ConnectionManager.init()))
                    ConnectionManager.init().remove_peer(addr)
                    logger.debug("handle_socket closed. connections after %s" % str(ConnectionManager.init()))
                    return
                message += part_msg

            # logger.debug("handle_socket receive message from addr %s" % str(addr))
            context = Context(peer_id, addr, ONLY_FROM_TCP_SERVER, sock, nonce)
            process_message_handler(context, message)
            # process_message_handler modify context.id
            if peer_id == MPTN_MAX_ID and context.id != MPTN_MAX_ID:
                ConnectionManager.init().add_peer(addr, Peer(socket=sock, id=context.id, address=addr))
                peer_id = context.id
                logger.debug("handle_socket update context id %s 0x%X from addr %s"%(ID_TO_STRING(context.id), context.id, str(addr)))

        except Exception as e:
            logger.error("handle_socket addr=%s, nonce=%s, size_string=%s, size=%d, message is \n%s\nerror=%s\n%s" % (str(addr),
                    str(map(ord, nonce)), str(map(ord, size_string)), size,
                    str(formatted_print(split_packet_to_list(message))), str(e),
                    traceback.format_exc()
                    )
                )
            logger.debug("handle_socket error before %s" % str(ConnectionManager.init()))
            ConnectionManager.init().remove_peer(addr)
            logger.debug("handle_socket error after %s" % str(ConnectionManager.init()))
            return

        gevent.sleep(0)

def reconnect(address):
    sock = None
    for i in xrange(CONNECTION_RETRIES):
        try:
            sock = socket.create_connection(address)#, NETWORK_TIMEOUT)
            return sock
        except IOError as e:
            logger.error('reconnect to %s with error=%s' % (str(address), str(e)))
        # gevent.sleep(3)
    else:
        if sock is not None: sock.close()
        return None

def socket_send(context, dest_id, message, expect_reply=False):
    if dest_id is None:
        logger.error("socket_send dest ID %s is not valid"%ID_TO_STRING(dest_id))
        return None

    if context is not None and context.direction == ONLY_FROM_TCP_SERVER and context.id == dest_id:
        # logger.debug("socket_send reuses socket since context ID is the same as dest_id %s" % str(dest_id))
        next_hop_id = dest_id
        address = context.address
        sock = context.socket
        nonce = context.nonce

    else:
        next_hop = find_nexthop_for_id(dest_id)
        # logger.debug("socket_send next_hop is %s" % str(next_hop))
        if next_hop is None:
            logger.error("socket_send next hop for dest ID %s 0x%X cannot be found"%(ID_TO_STRING(dest_id), dest_id))
            return None

        next_hop_id = next_hop.id
        address = next_hop.tcp_address

        sock = ConnectionManager.init().get_peer_by_id(next_hop_id)
        nonce = os.urandom(MPTN_TCP_NONCE_SIZE)

    if sock is None:
        # logger.debug("socket_send no socket found for ID %s"%ID_TO_STRING(next_hop_id))
        sock = reconnect(address)
        if sock is None:
            # logger.error("socket_send cannot re-setup socket for next_hop_id=%s addr=%s msg is\n%s" % (ID_TO_STRING(next_hop_id), str(address), formatted_print(split_packet_to_list(message))))
            return
        try:
            sock.send(self_id_net_endian_string)
        except Exception as e:
            logger.error("socket_send self_id_net_endian_string error=%s. addr=%s, self_id_net_endian_string=%s, nonce=%s, message is\n%s\nerror=%s\n%s" % (str(address), ID_TO_STRING(self_id_net_endian_string),
                    str(map(ord, nonce)),
                    str(formatted_print(split_packet_to_list(message))),
                    str(e), traceback.format_exc()
                    )
                )
            return
        gevent.spawn(socket_recv, sock, address, next_hop_id)
        ConnectionManager.init().add_peer(address, Peer(socket=sock, id=next_hop_id, address=address))
        gevent.sleep(0)

    # logger.debug("socket_send message %s to ID %s" % (str(message), ID_TO_STRING(next_hop_id)))
    size = 0
    try:
        sock.send(nonce)
        size = struct.pack("!L",socket.htonl(len(message)))
        sock.send(size)
        sock.sendall(message)
    except Exception as e:
        logger.error("socket_send nonce addr=%s, self_id_net_endian_string=%s, nonce=%s, message is\n%s\nerror=%s\n%s" % (str(address),
                ID_TO_STRING(self_id_net_endian_string),
                str(map(ord, nonce)),
                str(formatted_print(split_packet_to_list(message))),
                str(e), traceback.format_exc()
                )
            )
        ConnectionManager.init().remove_peer(address)
        return None

    if not expect_reply: return None

    callback = AsyncResult()
    ConnectionManager.init().add_nonce(nonce, NonceCallback(dest_id, callback))

    if context is None:
        while not callback.ready():
            gevent.sleep(0)

    return callback.get()

transport_if_send_handler = None
def set_transport_if_send(handler):
    global transport_if_send_handler
    transport_if_send_handler = handler

def transport_if_send(address, message):
    if transport_if_send == None: return (False, None)
    else: return transport_if_send_handler(address, message)

'''
DBDict class: a DB on disk with a dictionary interface
From mail.python.org/pipermail/python-list
'''

try:
   import cPickle as pickle
except:
   import pickle

import os, os.path
import UserDict
import sqlite3

def to_db_type(value):
    """
    If value's type is supported natively in SQLite, return value.
    Otherwise, return a pickled representation.
    """
    if value is None or isinstance(value, (int, long, float, basestring)):
        return value
    else:
        return buffer(pickle.dumps(value))

def from_db_type(value):
    """
    Converts a value from the database to a Python object.
    """
    if isinstance(value, buffer):
        return pickle.loads(str(value))
    else:
        return value

class DBDict(UserDict.DictMixin):
    """
    Shelf implementation using an SQLite3 database.
    """
    def __init__(self, filename):
        if not os.path.isfile(filename):
            self._database = sqlite3.connect(filename)
            self._database.execute("CREATE TABLE IF NOT EXISTS Shelf "
                               "(Key TEXT PRIMARY KEY NOT NULL, Value BLOB)")
        else:
            self._database = sqlite3.connect(filename)
        self._database.text_factory = str
        self._open = True

    def __del__(self):
        self.close()

    def __getitem__(self, key):
        row = self._database.execute("SELECT Value FROM Shelf WHERE Key=?",[key]).fetchone()
        if row:
            return from_db_type(row[0])
        else:
            raise KeyError(key)

    def __setitem__(self, key, value):
        self._database.execute("INSERT OR REPLACE INTO Shelf VALUES (?, ?)",[key, to_db_type(value)])
        self._database.commit()

    def __delitem__(self, key):
        if not self._database.execute("SELECT Key FROM Shelf WHERE Key=?",[key]).fetchone():
            raise KeyError(key)
        self._database.execute("DELETE FROM Shelf WHERE Key=?", [key])
        self._database.commit()

    def keys(self):
        """Return a list of keys in the shelf."""
        return [row[0] for row in self._database.execute("SELECT Key FROM Shelf")]

    def close(self):
        """Commit changes and close the file."""
        if self._database is not None:
            self._database.commit()
            self._database.close()
            self._database = None