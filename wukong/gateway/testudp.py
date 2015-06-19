import socket
import struct
import sys
import mptnUtils as MPTN
import uuid
import ipaddress
if len(sys.argv) == 1:
	print "python tetsudp.py <gateway address> <my node address>:<my port> [info | ID <short address> | sendto <dest dotted node id > <my dotted node id> <payload> ]"
	sys.exit(-1)
hardcode_ip = sys.argv[1]
#hardcode_ip = "10.0.0.6"

if __name__ == "__main__":
    tcp_address = sys.argv[2].split(":")
    address = MPTN.ID_FROM_STRING(tcp_address[0])
    port = int(tcp_address[1])
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(("",port))
    if sys.argv[3] == 'info':
        p = struct.pack('BBBBBBBBBB', 0xAA,0x55,0,address&0xff,(address>>8)&0xff,(address>>16)&0xff,(address>>24)&0xff,port%256,port/256,2)

        HOST = socket.gethostbyname(hardcode_ip)
        sock.sendto(p,(HOST,5775))
        p = sock.recv(1000)
        if ord(p[0]) == 0xAA and ord(p[1]) == 0x55:
            print "Node ID is %d" % ord(p[2])
        else:
            print "Unknown message:",[p]

    elif sys.argv[3] == 'ID':
        dest_id = MPTN.MASTER_ID
        src_id = MPTN.MPTN_MAX_ID
        msg_type = MPTN.MPTN_MSGTYPE_IDREQ
        message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, uuid.uuid4().bytes)
        p = struct.pack('BBBBBBBBBB', 0xAA,0x55,int(sys.argv[4]),address&0xff,(address>>8)&0xff,(address>>16)&0xff,(address>>24)&0xff,port%256,port/256,1)
        p = p+message

        HOST = socket.gethostbyname(hardcode_ip)
        sock.sendto(p,(HOST,5775))

        p = sock.recv(1000)
        dest_id, src_id, msg_type, payload = MPTN.extract_packet_from_str(p)
        if msg_type == MPTN.MPTN_MSGTYPE_IDACK and src_id == MPTN.MASTER_ID:
            src_id = dest_id
            print "Your ID is %d of which dotted format is %s" % (src_id, MPTN.ID_TO_STRING(src_id))
        else:
            print "Cannot get an ID"

    elif sys.argv[3] == "sendto":
        dest_id = MPTN.ID_FROM_STRING(sys.argv[4])
        src_id = MPTN.ID_FROM_STRING(sys.argv[5])
        payload = sys.argv[6]
        msg_type = MPTN.MPTN_MSGTYPE_FWDREQ

        message = MPTN.create_packet_to_str(dest_id, src_id, msg_type, payload)
        p = struct.pack('BBBBBBBBBB', 0xAA,0x55,src_id&0xff,address&0xff,(address>>8)&0xff,(address>>16)&0xff,(address>>24)&0xff,port%256,port/256,1)
        p = p+message

        HOST = socket.gethostbyname(hardcode_ip)
        sock.sendto(p,(HOST,5775))

        message = sock.recv(1000)
        dest_id, src_id, msg_type, payload = MPTN.extract_packet_from_str(message)
        if msg_type == MPTN.MPTN_MSGTYPE_FWDREQ:
            print "Payload from %X (%s) is %s" % (src_id, MPTN.ID_TO_STRING(src_id), map(ord, payload))
        else:
            print "Get invalid message %s" % MPTN.formatted_print(MPTN.split_packet_to_list(message))
