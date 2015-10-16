import socket
import sys
import gtwconfig as Config

class ProgressionServerClient:
    def __init__(self):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        except socket.err:
            print 'Failed to create socket'
            sys.exit()

    def send(self, deviceId,  portId, value):
        try:
            data = bytearray([(deviceId >> 24) & 0xff, (deviceId >> 16) & 0xff, (deviceId >> 8) & 0xff, deviceId & 0xff, portId / 256, portId % 256, value / 256, value % 256])
            self.sock.sendto(data, (Config.PSERVER_IP, Config.PSERVER_UDP_PORT))
        except socket.error, msg:
            print 'Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
            sys.exit()


def main(argv=None):
    if argv is None:
        argv = sys.argv

    client = ProgressionServerClient()
    # Test send data
    for i in range(10):
        client.send(1000, 100, 255)

if __name__ == '__main__':
    main()