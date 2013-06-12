#!/usr/local/bin/python
import reconfig_demo as demo
from wkpf.wuapplication import ChangeSets
from wkpf.wkpfcomm import getComm
from wkpf.wkpfcomm import WKPF_PROPERTY_TYPE_SHORT, WKPF_PROPERTY_TYPE_BOOLEAN, WKPF_PROPERTY_TYPE_REFRESH_RATE
from wkpf.transport import ZwaveAgent
from wkpf.mapper import first_of
from wkpf.models import WuNode, WuClassDef, WuClass, WuObject, WuComponent
from network_mode_test_fixture import fake_network_info
from random import randrange

REAL_DEPLOY = True
NETWORK_MODE = 2

## Testing Scenario:
# Testing where a node 32 in region 1 will be turned off

# Initially there are random amount of nodes in regions
# This generates the new configuration
def dummy_initial_configuration():
  data = {}
  for ind in range(demo.NUMBER_OF_REGIONS):
    region = ind+1
    data[str(region)] = randrange(3)
  return data

# This generates the old configuration which has one node in region 1, 2, and 3
def dummy_configuration():
  data = {}
  for ind in range(demo.NUMBER_OF_REGIONS):
    region = ind+1
    data[str(region)] = 0
  return data

# Generates a fake deployed application with WuComponent instances
def dummy_last_changesets(dummy_configuration, fake_network_info):
  wuclassdef = WuClassDef.find(id=demo.wuclass_id)

  dummy = ChangeSets([], [], [])
  for region, requirements in dummy_configuration.items():
    dummy.components.append(WuComponent(0, 
      str(region), int(requirements), 1, wuclassdef.name, 'demo_GH'))

  for region, nodes in fake_network_info.items():
    for component in dummy.components:
      if component.location == region and len(component.instances) < component.group_size:
        node = nodes[randrange(len(nodes))]
        wuobjects = node.wuobjects()
        component.instances.append(wuobjects[randrange(len(wuobjects))])
        break
  return dummy

def retrieve_fake_network_info():
  return fake_network_info(NETWORK_MODE)

def fakeDeploy(commands):
  for wuproperty in commands:
    print 'setting node', wuproperty.wuobject().wunode().id, 'on' if wuproperty.value else 'off'
    setFakeProperty(wuproperty)

def setFakeProperty(wuproperty):
  wuobject = wuproperty.wuobject()
  wuclassdef = wuobject.wuclass().wuclassdef()
  wunode = wuobject.wunode()
  value = wuproperty.value
  datatype = wuproperty.datatype
  number = wuproperty.wupropertydef().number

  if datatype == 'boolean':
    datatype = WKPF_PROPERTY_TYPE_BOOLEAN

  elif datatype == 'short':
    datatype = WKPF_PROPERTY_TYPE_SHORT

  elif datatype == 'refresh_rate':
    datatype = WKPF_PROPERTY_TYPE_REFRESH_RATE

  if datatype == WKPF_PROPERTY_TYPE_BOOLEAN:
    payload=[wuobject.port_number, wuclassdef.id/256,
    wuclassdef.id%256, number, datatype, 1 if value else 0]

  elif datatype == WKPF_PROPERTY_TYPE_SHORT or datatype == WKPF_PROPERTY_TYPE_REFRESH_RATE:
    payload=[wuobject.port_number, wuclassdef.id/256, wuclassdef.id%256, number, datatype, value/256, value%256]

  print "payload", payload

def dummy(cls):
  pass

if not REAL_DEPLOY:
  ZwaveAgent.__init__ = dummy
  demo.deploy = fakeDeploy

# New configuration
configuration = dummy_initial_configuration()
print "new config", configuration

# New application
new_changesets = demo.generate_demo_application(configuration)
#print "new application", new_changesets

network_info = retrieve_fake_network_info()
#print "network info", network_info

# Old dummy configuration
configuration = dummy_configuration()
#print "old config", configuration

# Previous application (node 32 in region 1 is on)
# configuration is used for setting up components (regions)
# network info is used to spawn wuobjects
last_changesets = dummy_last_changesets(configuration, network_info)
print "last deployed application", last_changesets

commands, last_changesets = first_of(new_changesets, network_info, last_changesets)

#print "commands", commands

demo.deploy(commands)
