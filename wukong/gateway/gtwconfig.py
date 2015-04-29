# import os
import logging

# from configobj import ConfigObj

# ROOT_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
# CONFIG_PATH = os.path.join(ROOT_PATH, 'wukong', 'config', 'gateway.cfg')
# config = ConfigObj(CONFIG_PATH)

LOG_LEVEL = logging.DEBUG

MASTER_IP = 'localhost'
MASTER_TCP_PORT = 9010
MASTER_ADDRESS = (MASTER_IP, MASTER_TCP_PORT)

SELF_TCP_SERVER_PORT = 9001

TRANSPORT_INTERFACE_TYPE = 'zwave'
# TRANSPORT_INTERFACE_TYPE = 'zigbee'
# TRANSPORT_INTERFACE_TYPE = 'udp'
TRANSPORT_INTERFACE_ADDR = '/dev/ttyACM0'
# TRANSPORT_INTERFACE_ADDR = '/dev/cu.usbmodem1411' # for Zwave on MacOSX
# TRANSPORT_INTERFACE_ADDR = 'eth0' # for UDP interface
# TRANSPORT_INTERFACE_ADDR = 'lo' # for UDP interface
# TRANSPORT_INTERFACE_ADDR = 'lo0' # for UDP interface on MacOSX

UNITTEST_MODE = False
UNITTEST_WAIT_SEC = 5


ENABLE_AUTONET = False
AUTONET_MAC_ADDR_LEN = 8


ENABLE_MONITOR = False
MONGODB_URL= "mongodb://140.112.170.32:27017/Wukong"

ENABLE_PROGRESSION = False
PSERVER_IP = 'localhost'
PSERVER_UDP_PORT = 8000