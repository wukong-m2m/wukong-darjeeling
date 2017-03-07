from gevent.queue import Queue, Empty
from gevent import monkey
monkey.patch_all()

from gevent import socket
import sys
import mptnUtils as MPTN
import gtwconfig as Config

# Pushing global monitoring message to progression server
WKPF_COMMAND_MONITOR                = 0xB5
# For system interceptors
WKPF_COMMAND_GET_LINK_COUNTER_R     = 0x61
WKPF_COMMAND_GET_DEVICE_STATUS_R    = 0x63
# For reconfiguration of a link
WKPF_COMMAND_CHANGE_LINK_R          = 0xA5
WKPF_COMMAND_SET_LOCK_R             = 0xA7
WKPF_COMMAND_RELEASE_LOCK_R         = 0xA9

class ProgressionService(object):
    def __init__(self):
        try:
            self._sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self._ip = Config.PSERVER_IP
            self._port = Config.PSERVER_UDP_PORT
            self._task = Queue()


            self._handlers = {}
            self._handlers[WKPF_COMMAND_GET_LINK_COUNTER_R] = self.handle_progression_message
            self._handlers[WKPF_COMMAND_GET_DEVICE_STATUS_R] = self.handle_progression_message
            self._handlers[WKPF_COMMAND_CHANGE_LINK_R] = self.handle_progression_message
            self._handlers[WKPF_COMMAND_RELEASE_LOCK_R] = self.handle_progression_message

        except socket.err:
            print 'Failed to create socket'
            sys.exit()

    def get_handlers(self):
        return self._handlers

    # still use MPTN message package to send payload to progression server
    def handle_progression_message(self, context, message):
        self._task.put_nowait((context, message))

    def serve_progression(self):
        while True:
            try:
                context, message = self._task.get(timeout=1)
            except Empty:
            if Config.ENABLE_PROGRESSION:
                send(message)

    def send(self, mptn):
        try:
            header = chr(0xaa) + chr(0x55) + chr(host_id) + struct.pack('<I', self._ip) + struct.pack('<H', self._port) + chr(raw_type) + chr(len(mptn))
            message = "".join(map(chr, payload))
            self.sock.sendto(header + message, self._ip, self._port)
        except socket.error, msg:
            print 'Error Code : ' + str(msg[0]) + ' Message ' + msg[1]

def main(argv=None):
    if argv is None:
        argv = sys.argv

    client = ProgressionService()
    # Test send data
    for i in range(10):
        client.send(1000, 100, 255)

if __name__ == '__main__':
    main()
