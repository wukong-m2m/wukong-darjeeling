import os, sys
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../..'))
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import test_environment
from master_server import *

#override some methods

def getFakeRoutingInformation():
  return test_environment.routing

def getFakeNodeInfos():
  return test_environment.simNodeInfos

def dummy(cls):
  pass

ZwaveAgent.__init__ = dummy
getComm().getRoutingInformation = getFakeRoutingInformation
getComm().getAllNodeInfos = getFakeNodeInfos

print "testing routing information"
print getComm().getRoutingInformation() # just testing

# TODO: make a app directory for test server...

ioloop.start() # starts our test server
