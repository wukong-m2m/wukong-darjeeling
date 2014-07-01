CONNECTION_RETRIES = 2
NETWORK_TIMEOUT = 3.0



RADIO_ADDRESS_LEN_ZW = 1
RADIO_ADDRESS_LEN_ZB = 2
RADIO_ADDRESS_LEN_IP = 4

STOP_MODE = (0, "STOP")
ADD_MODE = (1, "ADD")
DEL_MODE = (2, "DELTE")

MULT_PROTO_IP_PORT = 9001

# Acceptable payload format
# 4 bytes: destionation DID
# 4 bytes: source DID
# 1 byte: message type
# 1 byte: message subtype
# rest byte(s): payload
MULT_PROTO_LEN_DID              = 4
MULT_PROTO_LEN_MSG_TYPE         = 1
MULT_PROTO_LEN_MSG_SUBTYPE      = 1

MULT_PROTO_HEADER_FORMAT = (MULT_PROTO_LEN_DID, MULT_PROTO_LEN_DID, MULT_PROTO_LEN_MSG_TYPE, MULT_PROTO_LEN_MSG_SUBTYPE)
MULT_PROTO_DEST_BYTE_OFFSET     = 0
MULT_PROTO_SRC_BYTE_OFFSET      = MULT_PROTO_DEST_BYTE_OFFSET + MULT_PROTO_LEN_DID
MULT_PROTO_MSG_TYPE_BYTE_OFFSET = MULT_PROTO_SRC_BYTE_OFFSET + MULT_PROTO_LEN_DID
MULT_PROTO_MSG_SUBTYPE_BYTE_OFFSET = MULT_PROTO_MSG_TYPE_BYTE_OFFSET + MULT_PROTO_LEN_MSG_TYPE
MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET = MULT_PROTO_MSG_SUBTYPE_BYTE_OFFSET + MULT_PROTO_LEN_MSG_SUBTYPE


MULT_PROTO_MSG_TYPE_RAD         = 0x00
MULT_PROTO_MSG_SUBTYPE_RAD_ADD  = 0x00
MULT_PROTO_MSG_SUBTYPE_RAD_DEL  = 0x01
MULT_PROTO_MSG_SUBTYPE_RAD_STOP = 0x02
MULT_PROTO_MSG_SUBTYPE_RAD_POLL = 0x03
MULT_PROTO_MSG_SUBTYPE_RAD_ACK  = 0x04
MULT_PROTO_MSG_SUBTYPE_RAD_NAK  = 0x05

MULT_PROTO_MSG_TYPE_DID         = 0x01
MULT_PROTO_MSG_SUBTYPE_DID_REQ  = 0x00
MULT_PROTO_MSG_SUBTYPE_DID_UPD  = 0x01
MULT_PROTO_MSG_SUBTYPE_DID_OFFR = 0x02
MULT_PROTO_MSG_SUBTYPE_DID_ACK  = 0x03
MULT_PROTO_MSG_SUBTYPE_DID_NAK  = 0x04

MULT_PROTO_MSG_TYPE_RPC         = 0x02
MULT_PROTO_MSG_SUBTYPE_RPC_CMD  = 0x00
MULT_PROTO_MSG_SUBTYPE_RPC_REP  = 0x01

MULT_PROTO_MSG_TYPE_FWD         = 0x03
MULT_PROTO_MSG_SUBTYPE_FWD      = 0x00
MULT_PROTO_MSG_SUBTYPE_FWD_ACK  = 0x01
MULT_PROTO_MSG_SUBTYPE_FWD_NAK  = 0x02

MULT_PROTO_MSG_TYPE_PFX         = 0x04
MULT_PROTO_MSG_SUBTYPE_PFX_REQ  = 0x00
MULT_PROTO_MSG_SUBTYPE_PFX_UPD  = 0x01
MULT_PROTO_MSG_SUBTYPE_PFX_ACK  = 0x02
MULT_PROTO_MSG_SUBTYPE_PFX_NAK  = 0x03


import binascii
import struct
from gevent import socket

HEADER_FORMAT_STR = "!" + ''.join([{1:'B',2:'H',4:'I',8:'Q'}[i] for i in MULT_PROTO_HEADER_FORMAT])

def split_packet_header(message):
    ret = []
    for i in MULT_PROTO_HEADER_FORMAT:
        ret.append(message[:i])
        message = message[i:]
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

def create_mult_proto_header_to_str(dest_did, src_did, msg_type, msg_subtype):
    l = [dest_did, src_did, msg_type, msg_subtype]
    h = struct.pack(HEADER_FORMAT_STR, *l)
    return h

def extract_mult_proto_header_from_str(s):
    header = s[:MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET]
    return struct.unpack(HEADER_FORMAT_STR, header)

def special_recv(sock):
    size = sock.recv(struct.calcsize("!L"))
    size = socket.ntohl(struct.unpack("!L", size)[0])
    message = ""
    while len(message) < size:
        message += sock.recv(size - len(message))
    return message

def special_send(sock, message):
    size = struct.pack("!L",socket.htonl(len(message)))
    sock.send(size)
    sock.sendall(message)


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