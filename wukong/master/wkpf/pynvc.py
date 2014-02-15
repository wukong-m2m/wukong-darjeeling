import pyzwave
import pyzigbee
import sys
import os
from configuration import *


REPRG_DJ_OPEN                = 0x10
REPRG_DJ_OPEN_R              = 0x11
REPRG_DJ_WRITE               = 0x12
REPRG_DJ_WRITE_R             = 0x13
REPRG_DJ_COMMIT              = 0x14
REPRG_DJ_COMMIT_R            = 0x15
REPRG_DJ_REBOOT              = 0x16

REPRG_DJ_RETURN_OK                 = 0x00
REPRG_DJ_RETURN_REQUEST_RETRANSMIT = 0x01
REPRG_DJ_RETURN_TOO_LARGE          = 0x02
REPRG_DJ_RETURN_FAILED             = 0x03

REPRG_OPEN                   = 0x10
REPRG_OPEN_R                 = 0x11
REPRG_WRITE                  = 0x20
REPRG_WRITE_R_OK             = 0x21
REPRG_WRITE_R_RETRANSMIT     = 0x22
REPRG_COMMIT                 = 0x30
REPRG_COMMIT_R_OK            = 0x31
REPRG_COMMIT_R_RETRANSMIT    = 0x32
REPRG_COMMIT_R_FAILED        = 0x33

GETRUNLVL                    = 0x50
GETRUNLVL_R                  = 0x51
SETRUNLVL                    = 0x60
SETRUNLVL_R                  = 0x61

APPMSG                       = 0x80
APPMSG_R                     = 0x81

GROUP_NOTIFY_NODE_FAILURE    = 0x71


MULT_PROTO_DEST_BYTE_OFFSET     = 0
MULT_PROTO_SRC_BYTE_OFFSET      = 2
MULT_PROTO_MSG_TYPE_BYTE_OFFSET = 4
MULT_PROTO_MSG_SUBTYPE_BYTE_OFFSET = 5
MULT_PROTO_MSG_PAYLOAD_BYTE_OFFSET = 6

MULT_PROTO_MSG_TYPE_APP         = 0x00 # for application like profile framework
MULT_PROTO_MSG_TYPE             = 0xFF

MULT_PROTO_MSG_SUBTYPE_GID_REQ  = 0x00
MULT_PROTO_MSG_SUBTYPE_GID_OFFER= 0x01
MULT_PROTO_MSG_SUBTYPE_GID_ACK  = 0x02
MULT_PROTO_MSG_SUBTYPE_RPC_REQ  = 0x06
MULT_PROTO_MSG_SUBTYPE_RPC_REP  = 0x07
MULT_PROTO_MSG_SUBTYPE_FORWARD  = 0x0A

MASTER_GID = 0x0001
GATEWAY_GID = 0x0000

#WKPF message types here
WKPF_GET_WUCLASS_LIST        = 0x90
WKPF_GET_WUCLASS_LIST_R      = 0x91
WKPF_GET_WUOBJECT_LIST       = 0x92
WKPF_GET_WUOBJECT_LIST_R     = 0x93
WKPF_READ_PROPERTY	         = 0x94
WKPF_READ_PROPERTY_R         = 0x95
WKPF_WRITE_PROPERTY	         = 0x96
WKPF_WRITE_PROPERTY_R	     = 0x97
WKPF_REQUEST_PROPERTY_INIT   = 0x98
WKPF_REQUEST_PROPERTY_INIT_R = 0x99
WKPF_GET_LOCATION            = 0x9A
WKPF_GET_LOCATION_R          = 0x9B
WKPF_SET_LOCATION            = 0x9C
WKPF_SET_LOCATION_R          = 0x9D
WKPF_GET_FEATURES            = 0x9E
WKPF_GET_FEATURES_R          = 0x9F
WKPF_SET_FEATURE             = 0xA0
WKPF_SET_FEATURE_R           = 0xA1
WKPF_ERROR_R                 = 0xAF

DEBUG_TRACE_PART             = 0xB0
DEBUG_TRACE_FINAL            = 0xB2
LOGGING                      = 0xB4

APPMSG_STATUS_WAIT_ACK       = 0x00
APPMSG_STATUS_ACK            = 0x01
APPMSG_STATUS_BUSY           = 0x02

RUNLVL_VM                    = 0x02
RUNLVL_CONF                  = 0x03
RUNLVL_RESET                 = 0x04

# Keep this list in sync with wkpf_config.h
WKPF_FEATURE_LIGHT_SENSOR         = 0
WKPF_FEATURE_LIGHT_ACTUATOR       = 1
WKPF_FEATURE_NUMERIC_CONTROLLER   = 2
WKPF_FEATURE_NATIVE_THRESHOLD     = 3
WKPF_FEATURE_TEMPERATURE_HUMIDITY_SENSOR = 4

pymodule = 0

# WARNING:obsolete, use transport.py instead
def sendcmd(dest, cmd, payload=[], retries=3):
  global pymodule
  pymodule.receive(10) # Clear pending messages
  print 'sending command', cmd, 'with payload', payload, 'to dest', dest
  while retries >= 0:
    try:
      if pymodule == pyzwave:
        pymodule.send(dest, [0x88, cmd] + payload)
      else:
        pymodule.send(dest, [cmd] + payload)
    except Exception as e:
      print e
      print "=============IOError============ retries remaining:"
      print retries
      if retries <= 0:
        raise
      else:
        retries -= 1
    else:
      if cmd == APPMSG:
        src, ack = pymodule.receive(5000) # Receive ack of APPMSG, TODO: see if sending succeeded, check if src==dest
        print "APPMSG ACK:", ack
      return

def receive(waitmsec=1000):
  global pymodule
  return pymodule.receive(waitmsec)

# WARNING:obsolete, use transport.py instead
def checkedReceive(allowedReplies, waitmsec=1000, verify=None):
  global pymodule
  while True:
    src, reply = pymodule.receive(waitmsec)
    if reply == None:
      print "No reply received. One of", allowedReplies, "expected."
      return None
    if not reply[0] in allowedReplies:
      print "Incorrect reply received. One of", allowedReplies, "expected, but got:", reply
      print "Dropped message"
    else:
      # Correct type received, possibly need to verify
      if verify==None or verify(reply[0], reply[1:]):
        return src, reply
      else:
        print "Incorrect reply received. Message type correct, but didnt pass verification:", reply
        print "Dropped message"

# WARNING:obsolete, use transport.py instead
def sendWithRetryAndCheckedReceive(destination, command, allowedReplies, payload=[], waitmsec=1000, retries=0, quitOnFailure=False, verify=None):
  while retries >= 0:
    try:
      sendcmd(destination, command, payload)
      src, reply = checkedReceive(allowedReplies, waitmsec, verify=verify)
      if not reply == None:
        return src, reply
    except:
      pass
    retries -= 1
  if quitOnFailure:
    print "=====FAIL====="
    print "No reply received for command:"
    print command
    print "payload:"
    print payload
    print "Aborting"
    quit()
  else:
    return None, None

# WARNING:obsolete, use transport.py instead
def init(option, debug=False):
    global pymodule
    if option == 'zwave' or option == 0:
        try:
            pyzwave.init(ZWAVE_GATEWAY_IP)
            pymodule = pyzwave
        except IOError as e:
            print e
            return None
    elif option == 'zigbee' or option == 1:
        pyzigbee.init()
        pymodule = pyzigbee
    print 'pynvc debugging'
    pymodule.setdebug(debug)
    return True

# WARNING:obsolete, use transport.py instead
def add():
    global pymodule
    pymodule.add()
    return 0

# WARNING:obsolete, use transport.py instead
def delete():
    global pymodule
    pymodule.delete()
    return 0

# WARNING:obsolete, use transport.py instead
def stop():
    global pymodule
    pymodule.stop()
    return 0

# WARNING:obsolete, use transport.py instead
def poll():
    global pymodule
    status = pymodule.poll()
    return status

#Sen 12.8.7
#result structure (self_id, total_nodes(include self), node_1_id, node_2_id.....)
# WARNING:obsolete, use transport.py instead
def discovery():
    global pymodule
    return pymodule.discover()

# WARNING:obsolete, use transport.py instead
def discoverNodes():
	node_lst = discover()
	gateway_id = node_lst[0]
	node_lst = node_lst[2:]
	node_lst.remove(gateway_id)
	print tuple(node_lst)
	return tuple(node_lst)
#  return (1, 3) # TODO: implement network discovery here
