import os
import logging

from configobj import ConfigObj

ROOT_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
CONFIG_PATH = os.path.join(ROOT_PATH, 'wukong', 'config', 'gateway.cfg')
config = ConfigObj(CONFIG_PATH)

MASTER_IP = 'localhost'
MASTER_TCP_PORT = 9010
SELF_IP_INTERFACE = config.get("SELF_IP_INTERFACE", 'lo')
#SELF_IP_INTERFACE = config.get("SELF_IP_INTERFACE", 'lo0') # for MacOSX
SELF_TCP_PORT = config.get('SELF_TCP_PORT', 9001)
CONNECTION_RETRIES = 2
NETWORK_TIMEOUT = 3.0
ENABLE_AUTONET = False
AUTONET_MAC_ADDR_LEN = 8
UNITTEST_MODE = True
UNITTEST_WAIT_SEC = 5

LOG_LEVEL = logging.DEBUG

TRANSPORT_DEV_ADDR = config.get('DEV_ADDR', '/dev/ttyACM0')
# TRANSPORT_DEV_ADDR = config.get('DEV_ADDR', '/dev/cu.usbmodem1411') # for MacOSX
# TRANSPORT_DEV_ADDR = config.get('DEV_ADDR', 'lo:9000')

TRANSPORT_DEV_TYPE = config.get('GATEWAY_TYPE', 'zwave')
# TRANSPORT_DEV_TYPE = config.get('GATEWAY_TYPE', 'zigbee')
# TRANSPORT_DEV_TYPE = config.get('GATEWAY_TYPE', 'udp')