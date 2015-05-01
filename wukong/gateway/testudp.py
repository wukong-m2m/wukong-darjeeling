import socket
import struct
import sys
import mptnUtils
import uuid

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

if sys.argv[0] == 'info':
    p = struct.pack('BBBBBBBBBB', 0xAA,0x55,2,192,168,2,133,5000/256,5000%256,2)

    HOST = socket.gethostbyname('192.168.2.133')
    sock.sendto(p,(HOST,5775))
elif sys.argv[1] == 'ID':
    p = struct.pack('BBBBBBBBBB', 0xAA,0x55,2,192,168,2,133,5775/256,5775%256,1)
    mptnp = struct.pack('IIB',0,0xffffffff,mptnUtils.MPTN_MSGTYPE_IDREQ)
    p = p + mptnp + uuid.uuid1().bytes

    HOST = socket.gethostbyname('192.168.2.133')
    sock.sendto(p,(HOST,5775))
    p = sock.recv(1000)
    print [p]
