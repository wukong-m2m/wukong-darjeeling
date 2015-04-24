RADIO_ADDRESS_LEN_ZW = 1
RADIO_ADDRESS_LEN_ZB = 2
RADIO_ADDRESS_LEN_IP = 4

STOP_MODE = (0, "STOP")
ADD_MODE = (1, "ADD")
DEL_MODE = (2, "DELTE")

MASTER_ID = 0
MPTN_UDP_PORT = 9002

# Acceptable payload format
# 4 bytes: destionation DID
# 4 bytes: source DID
# 1 byte: message type
# rest byte(s): payload
MPTN_LEN_DID                = 4
MPTN_LEN_MSGTYPE            = 1

MPTN_HEADER_FORMAT          = (MPTN_LEN_DID, MPTN_LEN_DID, MPTN_LEN_MSGTYPE)
MPTN_DEST_BYTE_OFFSET       = 0
MPTN_SRC_BYTE_OFFSET        = MPTN_DEST_BYTE_OFFSET + MPTN_LEN_DID
MPTN_MSGTYPE_BYTE_OFFSET    = MPTN_SRC_BYTE_OFFSET + MPTN_LEN_DID
MPTN_PAYLOAD_BYTE_OFFSET    = MPTN_MSGTYPE_BYTE_OFFSET + MPTN_LEN_MSGTYPE

MPTN_MSGTYPE_GWDISCOVER     = 0
MPTN_MSGTYPE_GWOFFER        = 1
MPTN_MSGTYPE_IDREQ          = 2
MPTN_MSGTYPE_IDACK          = 3
MPTN_MSGTYPE_IDNAK          = 4
MPTN_MSGTYPE_GWIDREQ        = 5
MPTN_MSGTYPE_GWIDACK        = 6
MPTN_MSGTYPE_GWIDNAK        = 7

MPTN_MSGTYPE_RTPING         = 8
MPTN_MSGTYPE_RTACK          = 9

MPTN_MSGTYPE_RPCCMD         = 16
MPTN_MSGTYPE_RPCREP         = 17

MPTN_MSGTYPE_FWDREQ         = 24
MPTN_MSGTYPE_FWDACK         = 25
MPTN_MSGTYPE_FWDNAK         = 26

import binascii
import struct
from gevent import socket

import logging
import gtwconfig as CONFIG
logging.basicConfig(level=CONFIG.LOG_LEVEL)
logger = logging.getLogger( __name__ )


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
    if len(s) < MPTN_PAYLOAD_BYTE_OFFSET: return None
    header, payload = s[:MPTN_PAYLOAD_BYTE_OFFSET], s[MPTN_PAYLOAD_BYTE_OFFSET:]
    if payload == "": payload = None
    return struct.unpack(HEADER_FORMAT_STR, header)+(payload,)

MPTN_TCP_PACKET_SIZE = struct.calcsize("!L")
MPTN_TCP_NONCE_SIZE = struct.calcsize("!4B")
def recv(sock):
    try:
        nonce = sock.recv(MPTN_TCP_NONCE_SIZE)
        size = sock.recv(MPTN_TCP_PACKET_SIZE)
        size = socket.ntohl(struct.unpack("!L", size)[0])
        message = ""
        while len(message) < size:
            message += sock.recv(size - len(message))

    except socket.timeout:
        logger.error("socket timeout. addr=%s msg=%s" % (str(address), message))
        return (None, None)
    except socket.error as e:
        logger.error("socket error=%s addr=%s msg=%s" % (str(e), str(address), message))
        return (None, None)
    except struct.error as e:
        logger.error("struct error=%s addr=%s msg=%s" % (str(e), str(address), message))
        return (None, None)
    except Exception as e:
        logger.error("unknown error=%s addr=%s msg=%s" % (str(e), str(address), message))
        return (None, None)

    return message, nonce

def send(sock, message, nonce):
    try:
        sock.send(nonce)
        size = struct.pack("!L",socket.htonl(len(message)))
        sock.send(size)
        sock.sendall(message)

    except socket.timeout:
        logger.error("socket timeout. addr=%s msg=%s" % (str(address), message))
        return False
    except socket.error as e:
        logger.error("socket error=%s addr=%s msg=%s" % (str(e), str(address), message))
        return False
    except struct.error as e:
        logger.error("struct error=%s addr=%s msg=%s" % (str(e), str(address), message))
        return False
    except Exception as e:
        logger.error("unknown error=%s addr=%s msg=%s" % (str(e), str(address), message))
        return False

    return True

import gevent
from gevent.lock import RLock
from gevent.event import AsyncResult
import os
class MPTNConnectionManager(object):
    _manager = None
    @classmethod
    def init(cls):
        if not cls._manager:
            cls._manager = MPTNConnectionManager()
        return cls._manager

    def __init__(self):
        self.mptn_lock = RLock()
        self.mptn_connections = {}
        self.nonce_lock = RLock()
        self.nonce_cache = {}

    def add_peer(self, mptn_id, addr, sock):
        with self.mptn_lock:
            if mptn_id in self.mptn_connections:
                self.remove_peer(mptn_id, addr)
            self.mptn_connections[mptn_id] = (addr, sock)

    def remove_peer(self, mptn_id, addr):
        with self.mptn_lock:
            old_addr, old_sock = self.mptn_connections.pop(mptn_id)
            if old_addr[0] != addr[0] or old_addr[1] != addr[1]:
                logger.debug("connection with old IP tuple %s is replaced with different one %s" % (str(old_addr), str(addr)))
            try:
                old_sock.close()
            except Exception as e:
                logger.error("socket addr=%s closed with error=%s" % (str(old_addr), str(e)))
            finally:
                old_sock = None
                old_addr = None
                del self.mptn_connections[mptn_id]

    def get_peer_socket(self, mptn_id):
        with self.mptn_lock:
            return self.mptn_connections.get(mptn_id, (None, None))

    def add_nonce(self, mptn_id, nonce, listener):
        with self.nonce_lock:
            if nonce in self.nonce_cache:
                logger.error("nonce collision occurs")
                old_id, old_listener = self.nonce_cache[nonce]
                old_listener.set(None)
                del self.nonce_cache[nonce]
                old_id = None
                old_listener = None
            self.nonce_cache[nonce] = (mptn_id, listener)

    def pop_nonce(self, nonce):
        with self.nonce_lock:
            if nonce not in self.nonce_cache: return (None, None)
            return self.nonce_cache.pop(nonce)

def get_manager():
    return MPTNConnectionManager.init()

def add_peer(mptn_id, addr, sock):
    get_manager().add_peer(mptn_id, addr, sock)

def remove_peer(mptn_id, addr):
    get_manager().remove_peer(mptn_id, addr)

def handle_reply_message(context, dest_id, src_id, msg_type, payload):
    logger.debug("connection re-setup")
    sock, addr, nonce = context
    mptn_id, listener = get_manager().pop_nonce(nonce)
    if mptn_id != src_id: logger.error("reply message unmatched")
    listener.set((dest_id, src_id, msg_type, payload))

def send_and_wait_for_reply(dest_id, address, message):
    addr, sock = get_manager().get_peer_socket(dest_id)
    if sock is None:
        for i in xrange(CONFIG.CONNECTION_RETRIES):
            try:
                sock = socket.create_connection(address, CONFIG.NETWORK_TIMEOUT)
                break
            except IOError as e:
                logger.error('failed to connect to %s:%s error=%s' % (address[0], address[1], str(e)))
            gevent.sleep(1)
        else:
            if sock is not None: sock.close()
            sock = None
            return None

    nonce = os.urandom(MPTN_TCP_NONCE_SIZE)
    if not send(sock, message, nonce): return None

    listener = AsyncResult()
    get_manager().add_nonce(dest_id, nonce, listener)
    return listener.get()

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