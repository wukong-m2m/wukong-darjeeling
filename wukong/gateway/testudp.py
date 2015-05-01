import socket
import struct
import sys
import mptnUtils as MPTN
import uuid

#hardcode_ip = '192.168.2.133'
hardcode_ip = "10.0.0.6"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(("",5775))

if sys.argv[0] == 'info':
    p = struct.pack('BBBBBBBBBB', 0xAA,0x55,2,192,168,2,133,5000/256,5000%256,2)

    HOST = socket.gethostbyname(hardcode_ip)
    sock.sendto(p,(HOST,5775))

elif sys.argv[1] == 'ID':
    dest_id = MPTN.MASTER_ID
    src_id = MPTN.MPTN_MAX_ID
    msg_type = MPTN.MPTN_MSGTYPE_IDREQ
    message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, uuid.uuid4().bytes)
    p = struct.pack('BBBBBBBBBB', 0xAA,0x55,2,192,168,2,133,5775/256,5775%256,1)
    p = p+message

    HOST = socket.gethostbyname(hardcode_ip)
    sock.sendto(p,(HOST,5775))

    p = sock.recv(1000)
    print [p]
