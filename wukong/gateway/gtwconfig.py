# import os
import logging

# from configobj import ConfigObj

# ROOT_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
# CONFIG_PATH = os.path.join(ROOT_PATH, 'wukong', 'config', 'gateway.cfg')
# config = ConfigObj(CONFIG_PATH)

LOG_LEVEL = logging.ERROR

# MASTER_IP = '192.168.4.4'
MASTER_IP = '10.0.0.9'
MASTER_TCP_PORT = 9010
MASTER_ADDRESS = (MASTER_IP, MASTER_TCP_PORT)

SELF_TCP_SERVER_PORT = 9001

# TRANSPORT_INTERFACE_TYPE = 'zwave'
# TRANSPORT_INTERFACE_TYPE = 'zigbee'
TRANSPORT_INTERFACE_TYPE = 'udp'
# TRANSPORT_INTERFACE_ADDR = '/dev/ttyACM0'
# TRANSPORT_INTERFACE_ADDR = '/dev/cu.usbmodem1421' # for Zwave on MacOSX
# TRANSPORT_INTERFACE_ADDR = 'wlan0' # for UDP interface
# TRANSPORT_INTERFACE_ADDR = 'lo' # for UDP interface
TRANSPORT_INTERFACE_ADDR = 'eth0' # for UDP interface on MacOSX

UNITTEST_MODE = False
UNITTEST_WAIT_SEC = 5


ENABLE_AUTONET = False
AUTONET_MAC_ADDR_LEN = 8


ENABLE_MONITOR = False
MONGODB_URL= "mongodb://140.112.170.32:27017/Wukong"

ENABLE_PROGRESSION = False
PSERVER_IP = 'localhost'
PSERVER_UDP_PORT = 8000

# System represent the instance of a wukong system which is the data source
# of a set of sensor data in graphite
SYSTEM_NAME = 'Wukong1'

# Graphite Access info
ENABLE_GRAPHITE = False
GRAPHITE_IP = 'localhost'
GRAPHITE_PORT = '2004'

ENABLE_CONTEXT = False
'''
XMPP_IP = '140.112.28.107'
XMPP_PORT = 10303
XMPP_SERVER = 'localhost'
XMPP_USER = 'user1'
XMPP_PASS = ''
XMPP_ROOM = 'pubsub@conference.'+XMPP_SERVER
XMPP_NICK = XMPP_ROOM+'/gateway0'
'''
XMPP_IP = '140.112.170.26'
XMPP_PORT = 5222
XMPP_SERVER = 'wukongdemac-mini.local'
XMPP_USER = 'admin'
XMPP_PASS = ''
XMPP_ROOM = 'monitor@conference.'+XMPP_SERVER
XMPP_NICK = XMPP_ROOM+'/gateway0'
#'''
