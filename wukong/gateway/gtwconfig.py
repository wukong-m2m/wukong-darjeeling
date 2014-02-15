import os
from configobj import ConfigObj

ROOT_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
CONFIG_PATH = os.path.join(ROOT_PATH, 'wukong', 'config', 'gateway.cfg')
config = ConfigObj(CONFIG_PATH)

MASTER_IP = config.get('MASTER_IP', None)
MASTER_GID = 0x0001

ZWAVE_ADDR = config.get('ZWAVE_ADDR', '/dev/ttyACM0')
TCP_PORT = int(config.get('TCP_PORT', 9000))
UDP_PORT = 9001

RT_IF_TYPE_ZW = 1
RT_IF_TYPE_XB = 2
RT_IF_TYPE_IP = 3

LEN_ADDR_ZW = 1
LEN_ADDR_XB = 2
LEN_ADDR_IP = 4

# Acceptable payload format
# 2 bytes: destionation GID
# 2 bytes: source GID
# 1 byte: message type
# 1 byte: message subtype
# n byte(s): payload
MULT_PROTO_LEN_GID              = 2
MULT_PROTO_LEN_MSG_TYPE         = 1
MULT_PROTO_LEN_MSG_SUBTYPE      = 1
MULT_PROTO_DEST_BYTE_OFFSET     = 0
MULT_PROTO_SRC_BYTE_OFFSET      = MULT_PROTO_DEST_BYTE_OFFSET + MULT_PROTO_LEN_GID
MULT_PROTO_MSG_TYPE_BYTE_OFFSET = MULT_PROTO_SRC_BYTE_OFFSET + MULT_PROTO_LEN_GID
MULT_PROTO_MSG_SUBTYPE_BYTE_OFFSET = MULT_PROTO_MSG_TYPE_BYTE_OFFSET + MULT_PROTO_LEN_MSG_TYPE
MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET = MULT_PROTO_MSG_SUBTYPE_BYTE_OFFSET + MULT_PROTO_LEN_MSG_SUBTYPE

MULT_PROTO_MSG_TYPE_APP         = 0x00 # for application like profile framework
MULT_PROTO_MSG_TYPE             = 0xFF

MULT_PROTO_MSG_SUBTYPE_GID_REQ  = 0x00
MULT_PROTO_MSG_SUBTYPE_GID_OFFER= 0x01
MULT_PROTO_MSG_SUBTYPE_GID_ACK  = 0x02
MULT_PROTO_MSG_SUBTYPE_RPC      = 0x06

