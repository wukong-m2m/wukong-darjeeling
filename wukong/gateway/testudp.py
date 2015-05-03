import socket
import struct
import sys
import mptnUtils as MPTN
import uuid
import ipaddress
if len(sys.argv) == 1:
	print "python tetsudp.py <gateway address> <device address> [info | ID <short address]"
	sys.exit(-1)
hardcode_ip = sys.argv[1]
#hardcode_ip = "10.0.0.6"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(("",5775))
address = int(ipaddress.ip_address(sys.argv[2]))
if sys.argv[3] == 'info':
    p = struct.pack('BBBBBBBBBB', 0xAA,0x55,0,address&0xff,(address>>8)&0xff,(address>>16)&0xff,(address>>24)&0xff,5775/256,5775%256,2)

    HOST = socket.gethostbyname(hardcode_ip)
    sock.sendto(p,(HOST,5775))
    p = sock.recv(1000)
    print [p]

elif sys.argv[3] == 'ID':
    dest_id = MPTN.MASTER_ID
    src_id = MPTN.MPTN_MAX_ID
    msg_type = MPTN.MPTN_MSGTYPE_IDREQ
    message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, uuid.uuid4().bytes)
    p = struct.pack('BBBBBBBBBB', 0xAA,0x55,int(sys.argv[4]),address&0xff,(address>>8)&0xff,(address>>16)&0xff,(address>>24)&0xff,5775/256,5775%256,1)
    p = p+message

    HOST = socket.gethostbyname(hardcode_ip)
    sock.sendto(p,(HOST,5775))

    p = sock.recv(1000)
    print [p]
