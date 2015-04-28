import binascii
import struct
import os
import gevent
import ipaddress
from gevent.lock import RLock
from gevent.event import AsyncResult
from gevent import socket
import collections

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
Peer = collections.namedtuple("Peer", "socket, greenlet")

CONNECTION_RETRIES = 2
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
MPTN_UDP_PORT = 9002

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

def create_packet_to_str(dest_did, src_did, msg_type, payload):
    header = struct.pack(HEADER_FORMAT_STR, dest_did, src_did, msg_type)
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
        logger.error("set_self_id unknown error: %s" % str(e))

class ConnectionManager(object):
    _manager = None
    @classmethod
    def init(cls):
        if not cls._manager:
            cls._manager = ConnectionManager()
        return cls._manager

    def __init__(self):
        self.mptn_lock = RLock()
        self.mptn_connections = {} # map ID to a Peer (address, socket)
        self.nonce_lock = RLock()
        self.nonce_cache = {} # map nonce to a NonceCallback (ID, listener) or map ID to a list of nonces [nonce, ...]

    def __repr__(self):
        return "ConnectionManager:\n\tconnections:\n\t\t%s\n\tnonces:\n\t\t%s" % (
            str(self.mptn_connections), str(self.nonce_cache))

    def add_peer(self, mptn_id, peer):
        with self.mptn_lock:
            if mptn_id in self.mptn_connections: self.remove_peer(mptn_id)
            self.mptn_connections[mptn_id] = peer

    def remove_peer(self, mptn_id):
        with self.mptn_lock:
            if mptn_id is None:
                logger.error("remove_peer not for None")
                return

            if mptn_id not in self.mptn_connections:
                logger.error("remove_peer not for ID %s 0x%X" % (ID_TO_STRING(mptn_id), mptn_id))
                return

            peer = self.mptn_connections.pop(mptn_id)
            try:
                peer.socket.close()
            except Exception as e:
                logger.error("remove_peer socket for ID=%s closed with error=%s" % (ID_TO_STRING(mptn_id), str(e)))
            # finally:
            #     if peer.greenlet is not None: peer.greenlet.kill()

            logger.info("remove_peer done for ID %s 0x%X" % (ID_TO_STRING(mptn_id), mptn_id))

    def get_peer(self, mptn_id):
        with self.mptn_lock:
            return self.mptn_connections.get(mptn_id)

    def add_nonce(self, nonce, nncb):
        with self.nonce_lock:
            old_nncb = self.pop_nonce(nonce)
            if old_nncb is not None:
                logger.error("nonce collision occurs. nonce of ID %s's replaces that of old ID %s's" % (ID_TO_STRING(nncb.id), ID_TO_STRING(old_nncb.id)))
                old_nncb.callback.set(None)
                del old_nncb
            self.nonce_cache[nonce] = nncb

    def pop_nonce(self, nonce):
        with self.nonce_lock:
            if nonce not in self.nonce_cache: return None
            return self.nonce_cache.pop(nonce)

def handle_reply_message(context, dest_id, src_id, msg_type, payload):
    nncb = ConnectionManager.init().pop_nonce(context.nonce)

    if nncb is None:
        logger.error("handle_reply_message no nonce %s exists" % str(map(ord, context.nonce)))
        return

    if nncb.id != src_id:
        logger.error("handle_reply_message ID %s 0x%X not match nonce %s callback ID %s 0x%X" % (ID_TO_STRING(src_id), src_id, str(map(ord, context.nonce)), ID_TO_STRING(nncb.id), nncb.id))
        return

    nncb.callback.set((dest_id, src_id, msg_type, payload))

def handle_socket(sock, addr):
    logger.debug("handle_socket for sock %s and addr %s" % (str(sock), str(addr)))
    global process_message_handler
    neighbor_id = None
    while True:
        message = ""
        id_string = ""
        nonce = ""
        size_string = ""
        size = 0
        try:
            id_string = sock.recv(MPTN_ID_LEN)
            if id_string == "":
                logger.error("handle_socket closed when getting id_string. addr=%s, neighbor_id=%s, nonce=%s, size_string=%s, size=%d, message=\n%s" % (
                    str(addr), str(neighbor_id),
                    str(map(ord, nonce)), str(map(ord, size_string)), size,
                    str(formatted_print(split_packet_to_list(message)))
                    )
                )
                logger.debug("handle_socket closed when getting id_string. before %s" % str(ConnectionManager.init()))
                ConnectionManager.init().remove_peer(neighbor_id)
                logger.debug("handle_socket closed when getting id_string. after %s" % str(ConnectionManager.init()))
                return

            neighbor_id = socket.ntohl(struct.unpack("!L", id_string)[0])
            logger.debug("handle_socket for ID %s" % ID_TO_STRING(neighbor_id))
            nonce = sock.recv(MPTN_TCP_NONCE_SIZE)
            if nonce == "":
                logger.error("handle_socket closed when getting nonce. addr=%s, neighbor_id=%s, nonce=%s, size_string=%s, size=%d, message=\n%s" % (
                    str(addr), str(neighbor_id),
                    str(map(ord, nonce)), str(map(ord, size_string)), size,
                    str(formatted_print(split_packet_to_list(message)))
                    )
                )
                logger.debug("handle_socket closed when getting nonce. before %s" % str(ConnectionManager.init()))
                ConnectionManager.init().remove_peer(neighbor_id)
                logger.debug("handle_socket closed when getting nonce. after %s" % str(ConnectionManager.init()))
                return

            size_string = sock.recv(MPTN_TCP_PACKET_SIZE)
            if size_string == "":
                logger.error("handle_socket closed when getting size_string. addr=%s, neighbor_id=%s, nonce=%s, size_string=%s, size=%d, message=\n%s" % (
                    str(addr), str(neighbor_id),
                    str(map(ord, nonce)), str(map(ord, size_string)), size,
                    str(formatted_print(split_packet_to_list(message)))
                    )
                )
                logger.debug("handle_socket closed when getting nonce. before %s" % str(ConnectionManager.init()))
                ConnectionManager.init().remove_peer(neighbor_id)
                logger.debug("handle_socket closed when getting nonce. after %s" % str(ConnectionManager.init()))
                return

            size = socket.ntohl(struct.unpack("!L", size_string)[0])
            while len(message) < size:
                message += sock.recv(size - len(message))

        except socket.timeout as e:
            logger.error("handle_socket timeout error=%s. addr=%s, neighbor_id=%s, nonce=%s, size_string=%s, size=%d, message=\n%s" % (str(e), str(addr), str(neighbor_id),
                    str(map(ord, nonce)), str(map(ord, size_string)), size,
                    str(formatted_print(split_packet_to_list(message)))
                    )
                )
            logger.debug("handle_socket timeout error before %s" % str(ConnectionManager.init()))
            ConnectionManager.init().remove_peer(neighbor_id)
            logger.debug("handle_socket timeout error after %s" % str(ConnectionManager.init()))
            return
        except socket.error as e:
            logger.error("handle_socket error=%s. addr=%s, neighbor_id=%s, nonce=%s, size_string=%s, size=%d, message=\n%s" % (str(e), str(addr), str(neighbor_id),
                    str(map(ord, nonce)), str(map(ord, size_string)), size,
                    str(formatted_print(split_packet_to_list(message)))
                    )
                )
            logger.debug("handle_socket error neighbor_id before %s" % str(ConnectionManager.init()))
            ConnectionManager.init().remove_peer(neighbor_id)
            logger.debug("handle_socket error neighbor_id after %s" % str(ConnectionManager.init()))
            return
        except struct.error as e:
            logger.error("handle_socket struct error=%s. addr=%s, neighbor_id=%s, nonce=%s, size_string=%s, size=%d, message=\n%s" % (str(e), str(addr), str(neighbor_id),
                    str(map(ord, nonce)), str(map(ord, size_string)), size,
                    str(formatted_print(split_packet_to_list(message)))
                    )
                )
            logger.debug("handle_socket struct error before %s" % str(ConnectionManager.init()))
            ConnectionManager.init().remove_peer(neighbor_id)
            logger.debug("handle_socket struct error after %s" % str(ConnectionManager.init()))
            return
        except Exception as e:
            logger.error("handle_socket unknown error=%s addr=%s, neighbor_id=%s, nonce=%s, size_string=%s, size=%d, message=\n%s" % (str(e), str(addr), str(neighbor_id),
                    str(map(ord, nonce)), str(map(ord, size_string)), size,
                    str(formatted_print(split_packet_to_list(message)))
                    )
                )
            logger.debug("handle_socket unknown error before %s" % str(ConnectionManager.init()))
            ConnectionManager.init().remove_peer(neighbor_id)
            logger.debug("handle_socket unknown error after %s" % str(ConnectionManager.init()))
            return

        logger.debug("handle_socket receive message from addr %s" % str(addr))
        context = Context(neighbor_id, addr, ONLY_FROM_TCP_SERVER, sock, nonce)
        process_message_handler(context, message)
        if context.id != neighbor_id:
            logger.debug("handle_socket found different context id %s 0x%X from neighbor_id %s 0x%X"%(ID_TO_STRING(context.id), context.id, ID_TO_STRING(neighbor_id), neighbor_id))
            neighbor_id = context.id
            ConnectionManager.init().add_peer(neighbor_id, Peer(socket=sock, greenlet=None))
        else:
            if ConnectionManager.init().get_peer(neighbor_id) is None:
                ConnectionManager.init().add_peer(neighbor_id, Peer(socket=sock, greenlet=None))
                logger.debug("handle_socket cache neighbor_id %s 0x%X"%(ID_TO_STRING(neighbor_id), neighbor_id))
            logger.debug("handle_socket context id %s 0x%X is the same as neighbor_id %s 0x%X"%(ID_TO_STRING(context.id), context.id, ID_TO_STRING(neighbor_id), neighbor_id))
        gevent.sleep(0)

def reconnect(mptn_id, address):
    sock = None
    for i in xrange(CONNECTION_RETRIES):
        gevent.sleep(1)
        try:
            sock = socket.create_connection(address)#, NETWORK_TIMEOUT)
            return sock
        except IOError as e:
            logger.error('reconnect to %s with error=%s' % (str(address), str(e)))
    else:
        if sock is not None: sock.close()
        return None

def socket_send(context, dest_id, message, expect_reply=False):
    global self_id_net_endian_string
    if dest_id is None:
        logger.error("socket_send dest ID %s is not valid"%ID_TO_STRING(dest_id))
        return None

    if context is not None and context.direction == ONLY_FROM_TCP_SERVER and context.id == dest_id:
        logger.debug("socket_send reuses socket since context ID is the same as dest_id %s" % str(dest_id))
        next_hop_id = dest_id
        address = context.address
        sock = context.socket
        nonce = context.nonce

    else:
        next_hop = find_nexthop_for_id(dest_id)
        logger.debug("socket_send next_hop is %s" % str(next_hop))
        if next_hop is None:
            logger.error("socket_send next hop for dest ID %s 0x%X cannot be found"%(ID_TO_STRING(dest_id), dest_id))
            return None

        next_hop_id = next_hop.id
        address = next_hop.tcp_address

        peer = ConnectionManager.init().get_peer(next_hop_id)
        if peer is not None: sock = peer.socket
        else: sock = None
        nonce = os.urandom(MPTN_TCP_NONCE_SIZE)

    if sock is None:
        logger.debug("socket_send no old socket found for ID %s"%ID_TO_STRING(next_hop_id))
        sock = reconnect(next_hop_id, address)
        if sock is None:
            logger.error("socket_send cannot setup socket for next_hop_id=%s addr=%s msg is\n%s" % (ID_TO_STRING(next_hop_id), str(address), formatted_print(split_packet_to_list(message))))
            return

        nonce = os.urandom(MPTN_TCP_NONCE_SIZE)
        greenlet = gevent.spawn(handle_socket, sock, address)
        ConnectionManager.init().add_peer(next_hop_id, Peer(socket=sock, greenlet=greenlet))
        gevent.sleep(0)
    else:
        logger.debug("socket_send an old socket %s found for ID %s"%(str(sock), ID_TO_STRING(next_hop_id)))

    logger.debug("socket_send socket connected for ID %s"%ID_TO_STRING(next_hop_id))
    size = 0
    try:
        sock.send(self_id_net_endian_string)
        sock.send(nonce)
        size = struct.pack("!L",socket.htonl(len(message)))
        sock.send(size)
        sock.sendall(message)
    except socket.timeout as e:
        logger.error("socket_send socket timeout error=%s. addr=%s, self_id_net_endian_string=%s, nonce=%s, size=%s, message=\n%s" % (str(e), 
                str(address), ID_TO_STRING(self_id_net_endian_string),
                str(map(ord, nonce)), str(size),
                str(formatted_print(split_packet_to_list(message)))
                )
            )
        ConnectionManager.init().remove_peer(next_hop_id)
        return None
    except socket.error as e:
        logger.error("socket_send socket error=%s. addr=%s, self_id_net_endian_string=%s, nonce=%s, size=%s, message=\n%s" % (str(e), 
                str(address), ID_TO_STRING(self_id_net_endian_string),
                str(map(ord, nonce)), str(size),
                str(formatted_print(split_packet_to_list(message)))
                )
            )
        ConnectionManager.init().remove_peer(next_hop_id)
        return None
    except Exception as e:
        logger.error("socket_send socket error=%s. addr=%s, self_id_net_endian_string=%s, nonce=%s, size=%s, message=\n%s" % (str(e), 
                str(address), ID_TO_STRING(self_id_net_endian_string),
                str(map(ord, nonce)), str(size),
                str(formatted_print(split_packet_to_list(message)))
                )
            )
        ConnectionManager.init().remove_peer(next_hop_id)
        return None

    logger.debug("socket_send has sent message to ID %s"%ID_TO_STRING(next_hop_id))

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