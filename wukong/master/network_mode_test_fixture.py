#!/usr/local/bin/python
from wkpf.wuapplication import ChangeSets
from wkpf.wkpfcomm import getComm
from wkpf.wkpfcomm import WKPF_PROPERTY_TYPE_SHORT, WKPF_PROPERTY_TYPE_BOOLEAN, WKPF_PROPERTY_TYPE_REFRESH_RATE
from wkpf.transport import ZwaveAgent
from wkpf.mapper import first_of
from wkpf.models import WuNode, WuClassDef, WuClass, WuObject, WuComponent
import random

wuclass_id = 16

def fake_node(id, location, energy):
  global wuclass_id
  node = WuNode.find(id=id)

  if not node:
    node = WuNode.create(id, location, energy)
  else:
    node.location = location
    node.energy = energy
    node.save()

  #print 'Going into node', node.id

  # For this demo, we need to creat an wuobject for every node
  # And fake the property ourselves (defaul: on)
  wuclassdef = WuClassDef.find(id=wuclass_id)
  if not wuclassdef:
    raise Exception("can't find wuclass with id %d" % (wuclass_id))

  wuclass = WuClass.find(wuclassdef_identity=wuclassdef.identity,
      node_identity=node.identity)

  if not wuclass:
    #print 'Creating wuclass', wuclassdef.id
    wuclass = WuClass.create(wuclassdef, node, False)

  wuobject = WuObject.find(wuclass_identity=wuclass.identity)

  if not wuobject:
    #print 'Creating wuobject', wuclassdef.id
    # TODO: need to be certain about port number or reconfig will fail
    port_number = 4
    wuobject = WuObject.create(port_number, wuclass)

  # Right now creating a new WuObject will also creates WuProperties
  # automatically

  # Should create wuproperty here but need to discuss later if we should have
  # a procedure to retrieve property

  return node

def fake_network_info(mode):
  #print 'Fake network info', mode
  network_info_data = {}
  nodes = []

  if mode == 0:
    nodes.append(fake_node(3, '1', 10.0))
    nodes.append(fake_node(4, '2', 100.0))
    nodes.append(fake_node(5, '3', 1000.0))
  elif mode == 1:
    nodes.append(fake_node(2, '1', 30.0))
    nodes.append(fake_node(3, '2', 20.0))
    nodes.append(fake_node(4, '3', 40.0))
    nodes.append(fake_node(5, '1', 31.0))
    nodes.append(fake_node(6, '1', 29.0))
    nodes.append(fake_node(7, '3', 50.0))
    nodes.append(fake_node(8, '2', 1.0))
    nodes.append(fake_node(9, '2', 1000.0))
  elif mode == 2:
    nodes.append(fake_node(2, '1', 30.0))
    nodes.append(fake_node(3, '2', 20.0))
    nodes.append(fake_node(4, '3', 40.0))
    nodes.append(fake_node(5, '1', 31.0))
    nodes.append(fake_node(6, '1', 29.0))
    nodes.append(fake_node(7, '3', 50.0))
    nodes.append(fake_node(8, '6', 57.0))
    nodes.append(fake_node(9, '2', 979.0))
    nodes.append(fake_node(10, '3', 96.0))
    nodes.append(fake_node(11, '5', 986.0))
    nodes.append(fake_node(12, '13', 115.0))
    nodes.append(fake_node(13, '16', 984.0))
    nodes.append(fake_node(32, '2', 14.0))
    nodes.append(fake_node(21, '2', 987.0))
    nodes.append(fake_node(22, '10', 58.0))
    nodes.append(fake_node(23, '8', 970.0))
    nodes.append(fake_node(24, '9', 62.0))
    nodes.append(fake_node(25, '7', 1039.0))
    nodes.append(fake_node(26, '11', 44.35))
    nodes.append(fake_node(27, '12', 969.0))
    nodes.append(fake_node(28, '13', 16.0))
    nodes.append(fake_node(29, '14', 985.0))
    nodes.append(fake_node(30, '15', 72.0))
    nodes.append(fake_node(31, '16', 1204.0))
    nodes.append(fake_node(37, '12', 888.0))
    nodes.append(fake_node(39, '8', 1123.0))
    nodes.append(fake_node(43, '7', 929.0))
    nodes.append(fake_node(44, '6', 972.0))
    nodes.append(fake_node(45, '5', 1035.0))
    nodes.append(fake_node(46, '4', 961.0))
    nodes.append(fake_node(47, '11', 959.0))
    nodes.append(fake_node(48, '6', 963.0))
  elif mode == 3:
    nodes.append(fake_node(2, '16', 10.0))
    nodes.append(fake_node(44, '16', 1.0))
    nodes.append(fake_node(46, '12', 10.0))
    nodes.append(fake_node(23, '8', 5.0))
    nodes.append(fake_node(39, '8', 6.0))
  elif mode == 4:
    nodes.append(fake_node(43, '7', 929.0))
  else:
    pass

  for node in nodes:
    network_info_data.setdefault(node.location, []).append(node)

  print 'network', network_info_data

  return network_info_data
