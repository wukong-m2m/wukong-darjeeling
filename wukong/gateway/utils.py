import binascii
import gtwconfig as CONST
import struct

def dump(msg):
    """Receives all message parts from socket, printing each frame neatly"""
    print "----------------------------------------"
    for part in msg:
        print "[%03d]" % len(part), # Trailing comma suppresses newline
        try:
            print part.decode('ascii')
        except UnicodeDecodeError:
            print r"0x%s" % (binascii.hexlify(part).decode('ascii'))

def create_mult_proto_header_byte_list(dest_gid, src_gid, msg_type, msg_subtype):
    s = create_mult_proto_header_str(est_gid, src_gid, msg_type, msg_subtype)
    return [i for i in s]

def create_mult_proto_header_str(dest_gid, src_gid, msg_type, msg_subtype):
    m = {1:'B',2:'H',4:'I',8:'Q'}
    l = []
    f = "<"
    for i, length in [(dest_gid, CONST.MULT_PROTO_LEN_GID), (src_gid, CONST.MULT_PROTO_LEN_GID), (msg_type, CONST.MULT_PROTO_LEN_MSG_TYPE), (msg_subtype, CONST.MULT_PROTO_LEN_MSG_SUBTYPE)]:
        l.append(i)
        f += m[length]
    h = struct.pack(f, *l)
    return h

def extract_mult_proto_header_from_str(s):
    header = s[:MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET]
    m = {1:'B',2:'H',4:'I',8:'Q'}
    f = "<"
    for length in [CONST.MULT_PROTO_LEN_GID, CONST.MULT_PROTO_LEN_GID, CONST.MULT_PROTO_LEN_MSG_TYPE, CONST.MULT_PROTO_LEN_MSG_SUBTYPE]:
        f += m[length]
    return struct.unpack(f, header)