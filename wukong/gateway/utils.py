import binascii

def dump(msg):
    """Receives all message parts from socket, printing each frame neatly"""
    print "----------------------------------------"
    for part in msg:
        print "[%03d]" % len(part), # Trailing comma suppresses newline
        try:
            print part.decode('ascii')
        except UnicodeDecodeError:
            print r"0x%s" % (binascii.hexlify(part).decode('ascii'))


import socket
import fcntl
import struct

def get_ip_address(ifname):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    return socket.inet_ntoa(fcntl.ioctl(
        s.fileno(),
        0x8915,  # SIOCGIFADDR
        struct.pack('256s', ifname[:15])
    )[20:24])

def get_net_mask(ifname):
    return socket.inet_ntoa(fcntl.ioctl(
        socket.socket(socket.AF_INET, socket.SOCK_DGRAM), 
        35099, 
        struct.pack('256s', ifname)
    )[20:24])

'''
print get_ip_address('lo')
print get_net_mask('lo')
print get_ip_address('eth0')
print get_net_mask('eth0')
print get_ip_address('wlan0')
'''