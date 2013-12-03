import time
from wkpf.wkpfcomm import *

def setValue(id, value):
  getComm().zwave.send_raw(id, [0x20, 0x01, value,])
  print "set id %d port 1" %(id)
  time.sleep(0.5)
  getComm().zwave.send_raw(id, [0x60,0x0d,0x1,0x2,0x20, 0x01, value,])
  print "set id %d port 2" %(id)
  time.sleep(0.5)
  getComm().zwave.send_raw(id, [0x60,0x0d,0x1,0x3,0x20, 0x01, value,])
  print "set id %d port 3" %(id)
  time.sleep(0.5)

ids = [x for x in getComm().getNodeIds()[1:] if x != 1]

print "ids", ids

id = 19

print "setting id", id

while 1:
  setValue(id, 0)
  time.sleep(1)
  setValue(id, 255)
  setValue(id, 50)
  time.sleep(1)

