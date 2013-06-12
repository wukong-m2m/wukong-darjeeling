#!/usr/local/bin/python
# Author: Penn Su
#
# TODO
#
# * Make it multithread with control and model threads?

from gevent import monkey; monkey.patch_all()
import urllib2
import urllib
import httplib
import time
import sys
import threading
from threading import Thread, Lock
from xml.dom.minidom import parse, parseString
import network_mode_test_fixture as fake_network
from wkpf.wuapplication import WuApplication, ChangeSets
from wkpf.wkpfcomm import *
from wkpf.parser import *
from wkpf.models import *
from wkpf.mapper import first_of
from configuration import *

LOCATION_ROOT = 'demo_GH'
NUMBER_OF_REGIONS = 16

wuclass_id = 16
last_seen_id = 0
last_seen_configuration = {}
last_changesets = ChangeSets([], [], [])
regions = range(16)
server_url = '140.112.170.27'
network_info_url = 'http://140.112.170.27/network_info.xml'
configuration_url = 'http://140.112.170.27/configuration.xml'

# This makes sure all nodes are on (because of caching of value on flash)
def turn_all_on(network_info):
  print 'turning all on'
  comm = getComm()
  for nodes in network_info.values():
    for node in nodes:
      for wuobject in node.wuobjects():
        for wuproperty in wuobject.wuproperties():
          wuproperty.value = True # a hack
          comm.setProperty(wuproperty)

# Initially turn on a node in every region
def initial_configuration():
  global NUMBER_OF_REGIONS
  data = {}
  for ind in range(NUMBER_OF_REGIONS):
    region = ind+1
    data[str(region)] = 1
  return data

def get_id(node_dom):
  return int(node_dom.getElementsByTagName('wk-id')[0].firstChild.data)

def get_location(node_dom):
  return node_dom.getElementsByTagName('region')[0].firstChild.data

def get_energy(node_dom):
  return float(node_dom.getElementsByTagName('energy')[0].firstChild.data)

# Construct WuNode, WuClass and WuObjects here
def to_node(node_dom):
  global wuclass_id
  id = get_id(node_dom)
  location = get_location(node_dom)
  energy = get_energy(node_dom)
  neighbors = node_dom.getElementsByTagName('neighbor')[0].firstChild.data

  node = WuNode.find(id=id)

  if not node:
    node = WuNode.create(id, location, energy)
  else:
    node.location = location
    node.energy = energy
    node.save()

  print 'Going into node', node.id

  # For this demo, we need to creat an wuobject for every node
  wuclassdef = WuClassDef.find(id=wuclass_id)
  if not wuclassdef:
    raise Exception("can't find wuclass with id %d" % (wuclass_id))

  wuclass = WuClass.find(wuclassdef_identity=wuclassdef.identity,
      node_identity=node.identity)

  if not wuclass:
    print 'Creating wuclass', wuclassdef.id
    wuclass = WuClass.create(wuclassdef, node, False)

  wuobject = WuObject.find(wuclass_identity=wuclass.identity)

  if not wuobject:
    print 'Creating wuobject', wuclassdef.id
    # TODO: need to be certain about port number or reconfig will fail
    port_number = 4
    wuobject = WuObject.create(port_number, wuclass)

  # Should create wuproperty here but need to discuss later if we should have
  # a procedure to retrieve property

  return node

'''
def retrieve_network_info():
  print "fake network info"
  return fake_network.fake_network_info(3)
'''

def retrieve_network_info():
  print "Bootstraping/Updating network info"
  response = urllib2.urlopen(network_info_url)
  network_info = response.read()
  network_info_dom = parseString(network_info)
  nodes_dom = network_info_dom.getElementsByTagName('node')
  network_info_data = {}
  for node_dom in nodes_dom:
    node = to_node(node_dom)
    network_info_data.setdefault(node.location, []).append(node)
  return network_info_data

def retrieve_configuration():
  global last_seen_id
  global last_seen_configuration
  data = urllib.urlencode({'id': last_seen_id})
  response = urllib2.urlopen(configuration_url, data)
  configuration_info = response.read()
  configuration_info_dom = parseString(configuration_info)
  print 'Retriving configuration...'
  updates = configuration_info_dom.getElementsByTagName('update')[0].firstChild.data == 'true'
  if updates:
    last_seen_id = configuration_info_dom.getElementsByTagName('state')[0].getElementsByTagName('id')[0].firstChild.data
    # They have a typo in their field, it should be dd-output I think
    requirements = configuration_info_dom.getElementsByTagName('state')[0].getElementsByTagName('dd-ouput')[0].firstChild.data
    configuration = {}
    for ind in range(len(requirements)):
      region = ind+1
      configuration[str(region)] = int(requirements[ind])
    last_seen_configuration = configuration
    return configuration
  return None

# Generates previous deployed application
def generate_initial_changesets(network_info):
  global wuclass_id

  wuclassdef = WuClassDef.find(id=wuclass_id)
  changesets = ChangeSets([], [], [])
  for region in range(1, NUMBER_OF_REGIONS+1):
    node_requirement = 0
    if str(region) in network_info:
      node_requirement = len(network_info[str(region)])
    component = WuComponent(0, str(region), node_requirement, 1, wuclassdef.name,
        'demo_GH')
    changesets.components.append(component)

  # Pick all
  for region, nodes in network_info.items():
    for component in changesets.components:
      if component.location == region and len(component.instances) < component.group_size:
        for node in nodes:
          wuobjects = node.wuobjects()
          for wuobject in wuobjects:
            component.instances.append(wuobject)
        break

  print 'Generating previous deployed application...'
  return changesets

# Generates application with Components equal to the number of regions
# Empty regions have group_size 0
# This applciation has only WuComponents with empty instances, which
# will be filled by mapper
def generate_demo_application(configuration):
  global wuclass_id

  if not configuration:
    return None

  wuclassdef = WuClassDef.find(id=wuclass_id)
  changesets = ChangeSets([], [], [])
  for region in range(1, NUMBER_OF_REGIONS+1):
    node_requirement = 0
    if str(region) in configuration.keys():
      node_requirement = configuration[str(region)]
    component = WuComponent(0, str(region), node_requirement, 1, wuclassdef.name,
        'demo_GH')
    changesets.components.append(component)
  print 'Generating application...'
  return changesets

def deploy(commands):
  if not commands:
    return

  print 'Deploying...'
  comm = getComm()
  for wuproperty in commands:
    wunode = wuproperty.wuobject().wunode()
    success = False
    retries = 3
    for retry in range(retries, 0, -1):
      if comm.setProperty(wuproperty):
        success = True
        break
      print 'Cannot set property for wunode id %d, trying again...' % (wunode.id)
    if not success:
      print 'Set property for node id %d failed' % (wunode.id)
    gevent.sleep(0)

# According to Prof. Shih, it is best to split a real time program to stick with
# schedule with threads: one for control, the other for data models
# However, since gevent is incompatible with threads, we are monkey patching
# threads to make them behave like greenlets
def network_thread():
  global network_info, changesets, lock
  configuration = initial_configuration()

  while True:
    lock.acquire()
    network_info = retrieve_network_info()
    print 'network_info', network_info

    changesets = generate_demo_application(configuration)
    print 'after generate_demo_application'
    lock.release()

    time.sleep(10)

    configuration = retrieve_configuration()
    print 'after retrieve_configuration'

def control_thread():
  global changesets, network_info, lock
  last_changesets = None

  while True:
    if not changesets or not network_info:
      time.sleep(0.1)
      print 'waiting for changesets'
      continue

    if not last_changesets:
      last_changesets = generate_initial_changesets(network_info)

    # Prevent changesets and network info get changed during mapping
    lock.acquire()
    commands, last_changesets = first_of(changesets, network_info, last_changesets)
    print 'commands', commands
    for wuproperty in commands:
      print 'setting node', wuproperty.wuobject().wunode().id, 'on' if wuproperty.value else 'off'

    changesets = None # Reset so that it will not go into mapping after
    lock.release()

    time.sleep(0.1)

    deploy(commands)

    time.sleep(0.1)


Parser.parseLibrary(COMPONENTXML_PATH)
changesets = network_info = None
lock = Lock()
network_info = retrieve_network_info()
turn_all_on(network_info)

if __name__ == "__main__":
  control = Thread(target=control_thread)
  network = Thread(target=network_thread)

  network.start()
  control.start()

  for thread in threading.enumerate():
    if thread is not threading.currentThread():
      thread.join()
