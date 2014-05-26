import serial
import gevent

class AutoNet:
    def __init__(self, transport_add, transport_delete, transport_stop):
        self.port = serial.Serial(baudrate=38400, timeout=1.0, port="/dev/ttyUSB0")
        self._add_device = transport_add
        self._delete_device = transport_delete
        self._stop_learning = transport_stop

        def _read_number(self):
            c = port.read()
            if c != '':
                return ord(c)
            return 0

        def get_gateway_mac_address(self):
            while True:
                command = ReadNumber()
                if command == 1:
                    num = ReadNumber()
                    device_list = []
                    for i in xrange(num):
                        device_list.append(ReadNumber())
                    # TODO: mac_address should be 8 bytes!
                    gateway_mac_address = device_list[0]
                    break

        def serve_autonet(self):
            while True:
                command = ReadNumber()
                if command == 1:
                    num = ReadNumber()
                    device_list = []
                    for i in xrange(num):
                        device_list.append(ReadNumber())

                elif command == 2: # add
                    new_device_mac_address = ReadNumber()
                    self._add_device()

                elif command == 3: # delete
                    del_device_mac_address = ReadNumber()
                    self._delete_device()
                gevent.sleep(0.01)