import binascii
import mptn as MPTN
import struct
from gevent import socket

HEADER_FORMAT_STR = "!" + ''.join([{1:'B',2:'H',4:'I',8:'Q'}[i] for i in MPTN.MULT_PROTO_HEADER_FORMAT])

def split_packet_header(message):
    return [message[MPTN.MULT_PROTO_DEST_BYTE_OFFSET:MPTN.MULT_PROTO_SRC_BYTE_OFFSET], message[MPTN.MULT_PROTO_SRC_BYTE_OFFSET:MPTN.MULT_PROTO_MSG_TYPE_BYTE_OFFSET], message[MPTN.MULT_PROTO_MSG_TYPE_BYTE_OFFSET:MPTN.MULT_PROTO_MSG_SUBTYPE_BYTE_OFFSET], message[MPTN.MULT_PROTO_MSG_SUBTYPE_BYTE_OFFSET:MPTN.MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET]]

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
    header = s[:MPTN.MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET]
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