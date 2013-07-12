#!/usr/local/bin/python
# Author: Penn Su
#
# Changesets contain only components and wuobjects
# Commands contains wuproperties with correct value to set
# Configuration is used to generate changesets
# Mapped changesets contain wuobjects, the property value is not important as it
# is assumed to be on for mapped wuobject (check source code)

from gevent import monkey; monkey.patch_all()
import sys
import json
import urllib2
import urllib
import httplib
import time
import sys
import threading
from threading import Thread, Lock
from xml.dom.minidom import parse, parseString
import network_mode_test_fixture as fake_network
from network_mode_test_fixture import fake_network_info
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
changesets = None
network_info = None
lock = Lock()
global_network_info = None
regions = range(16)
server_url = '140.112.170.27'
network_init_url = 'http://140.112.170.27/network_init.xml'
network_info_url = 'http://140.112.170.27/network_info.xml'
configuration_url = 'http://140.112.170.27/configuration.xml'
status_update_url = 'http://140.112.170.27/status_update'

def timing(f):
  def wrap(*args):
    time1 = time.time()
    ret = f(*args)
    time2 = time.time()
    print '%s function took %0.3f s' % (f.func_name, (time2-time1))
    return ret
  return wrap

def send_status(data):
  request = urllib2.Request(status_update_url)
  request.add_data(data)
  response = urllib2.urlopen(request)

def init_status(status, network_info):
  data_dict = {}
  data_dict["wukong_status"] = {}
  d = data_dict["wukong_status"]
  '''
  for region, nodes in network_info.items():
    node_ids = [str(node.id) for node in nodes]
    key = "region" + region
    d[key] = ','.join(node_ids)
  '''
  d["status"] = status

  values = json.dumps(data_dict)
  data = urllib.urlencode(data_dict)
  send_status(data)

def update_status(status, changesets):
  data_dict = {}
  data_dict["wukong_status"] = {}
  d = data_dict["wukong_status"]
  for component in changesets.components:
    # Assuming instances all are in unique nodes
    nodes = [str(wuobject.wunode().id) for wuobject in component.instances]
    key = 'region' + component.location
    d[key] = ','.join(nodes)
  d['status'] = status

  values = json.dumps(data_dict)
  data = urllib.urlencode(data_dict)
  send_status(data)

# This makes sure all nodes are on (because of caching of value on flash)
@timing
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
  if not node_dom.getElementsByTagName('wk-id') or not node_dom.getElementsByTagName('wk-id')[0].firstChild:
    return False
  return int(node_dom.getElementsByTagName('wk-id')[0].firstChild.data)

def get_location(node_dom):
  return node_dom.getElementsByTagName('region')[0].firstChild.data

def get_energy(node_dom):
  if not node_dom.getElementsByTagName('energy'):
    return 0
  return float(node_dom.getElementsByTagName('energy')[0].firstChild.data)

def get_neighbors(node_dom):
  if not node_dom.getElementsByTagName('neighbor'):
    return []
  return node_dom.getElementsByTagName('neighbor')[0].firstChild.data

# Construct WuNode, WuClass and WuObjects here
def to_node(node_dom):
  global wuclass_id
  if get_id(node_dom) and get_id(node_dom) != 0:
    id = get_id(node_dom)
    location = get_location(node_dom)
    energy = get_energy(node_dom)
    #neighbors = get_neighbors(node_dom)

    node = WuNode.find(id=id)

    if not node:
      node = WuNode.create(id, location, energy)
    else:
      node.location = location
      node.energy = energy
      node.save()

    #print 'Going into node', node.id

    # For this demo, we need to creat an wuobject for every node
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

    # Should create wuproperty here but need to discuss later if we should have
    # a procedure to retrieve property

    return node
  return None

def setup_network_info(url):
  response = urllib2.urlopen(url)
  network_info = response.read()
  network_info_dom = parseString(network_info)
  nodes_dom = network_info_dom.getElementsByTagName('node')
  ret = {}
  for node_dom in nodes_dom:
    node = to_node(node_dom)
    if node:
      ret.setdefault(node.location, []).append(node)
  return ret

def to_node_zwave(node_id):
  global wuclass_id
  id = node_id

  node = WuNode.find(id=id)

  if not node:
    node = WuNode.create(id, 'whatever', 0.0)

  print 'Going into node', node.id

  # For this demo, we need to creat an wuobject for every node
  wuclassdef = WuClassDef.find(id=wuclass_id)
  if not wuclassdef:
    raise Exception("can't find wuclass with id %d" % (wuclass_id))

  wuclass = WuClass.find(wuclassdef_identity=wuclassdef.identity,
      node_identity=node.identity)

  if not wuclass:
    wuclass = WuClass.create(wuclassdef, node, False)

  wuobject = WuObject.find(wuclass_identity=wuclass.identity)

  if not wuobject:
    port_number = 4
    wuobject = WuObject.create(port_number, wuclass)

  return node

# Use zwave discovery list instead
def init_network_info_zwave():
  comm = getComm()
  ret = {}
  for node_id in comm.getNodeIds():
    node = to_node_zwave(node_id)
    if node:
      ret.setdefault(node.location, []).append(node)

  return ret

def init_network_info():
  global global_network_info
  print "Initializing network info"
  ret = setup_network_info(network_init_url)
  #print ret
  global_network_info = ret
  return ret

def update_network_info(initial_network_info):
  print "Updating network info"
  ret = setup_network_info(network_info_url)
  for location, nodes in initial_network_info.items():
    if location in ret:
      for node in nodes:
        for fnode in ret[location]:
          if fnode.id == node.id:
            node.energy = fnode.energy
            break
  return initial_network_info

def retrieve_network_info():
  print "Retrieve network info"
  ret = init_network_info()
  ret = update_network_info(ret)
  return ret

'''
def retrieve_network_info():
  print "fake network info"
  return fake_network.fake_network_info(3)
'''

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

# Modify changesets as reflected on the network info
# Remember changesets only reflect nodes that has property turned on
def reflect_on_actual_deployed_network_status(changesets, network_info):
  for component in changesets.components:
    # per component (location)
    if component.location in network_info:
      nodes = network_info[component.location]
      wuobjects = []
      for node in nodes:
        for wuobject in node.wuobjects():
          wuobjects.append(wuobject)
      component.instances = wuobjects
  return changesets

# Generates initial deployed application
def generate_initial_demo_application_with_components(network_info):
  global wuclass_id

  wuclassdef = WuClassDef.find(id=wuclass_id)
  changesets = ChangeSets([], [], [])
  for region in range(1, NUMBER_OF_REGIONS+1):
    region_s = str(region)

    node_requirement = 0
    if region_s in network_info:
      node_requirement = len(network_info[region_s])

    component = WuComponent(0, region_s, node_requirement, 1, wuclassdef.name, 'demo_GH')

    # Pick all
    if region_s in network_info:
      nodes = network_info[region_s]
      for node in nodes:
        for wuobject in node.wuobjects():
          component.instances.append(wuobject)

    changesets.components.append(component)

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

@timing
def deploy(commands):
  if not commands:
    return

  print 'Deploying nodes...'
  count = 0
  comm = getComm()
  for wuproperty in commands:
    wunode = wuproperty.wuobject().wunode()
    success = False
    retries = 3
    for retry in range(retries, 0, -1):
      if comm.setProperty(wuproperty):
        count += 1
        success = True
        break
      print 'Cannot set property for wunode id %d, trying again...' % (wunode.id)
    if not success:
      print 'Set property for node id %d failed' % (wunode.id)
    gevent.sleep(0)
  print 'Deployed to %d / %d nodes...' % (count, len(commands))

# According to Prof. Shih, it is best to split a real time program to stick with
# schedule with threads: one for control, the other for data models
# However, since gevent is incompatible with threads, we are monkey patching
# threads to make them behave like greenlets
def network_thread():
  global global_network_info, network_info, changesets, lock
  configuration = initial_configuration()

  while True:
    lock.acquire()
    network_info = retrieve_network_info()
    #print 'network_info', network_info

    changesets = generate_demo_application(configuration)
    lock.release()

    time.sleep(20)

    #configuration = retrieve_configuration()

def control_thread():
  global changesets, global_network_info, network_info, lock
  last_changesets = generate_initial_demo_application_with_components(global_network_info)
  #print 'last_changesets', last_changesets

  count = 0
  while True:
    sys.stdout.write('\r')
    if not changesets or not network_info:
      sys.stdout.write('.')
      sys.stdout.flush()
      time.sleep(0.1)
      continue
    sys.stdout.write('\n')

    # reflect on the actual deployed status
    #last_changesets = reflect_on_actual_deployed_network_status(last_changesets, network_info)

    # Prevent changesets and network info get changed during mapping
    lock.acquire()
    commands, last_changesets = first_of(changesets, network_info, last_changesets)
    print 'command'
    for wuproperty in commands:
      print '\tsetting node', wuproperty.wuobject().wunode().id, 'on' if wuproperty.value else 'off'

    update_status('WuKong is mapping', last_changesets)

    changesets = None # Reset so that it will not go into mapping after
    lock.release()

    time.sleep(0.1)

    deploy(commands)

    time.sleep(0.1)


Parser.parseLibrary(COMPONENTXML_PATH)
network_info = init_network_info_zwave()
turn_all_on(network_info)
time.sleep(10)
network_info = init_network_info()
init_status('WuKong is initializing', network_info)

if __name__ == "__main__":
  control = Thread(target=control_thread)
  network = Thread(target=network_thread)

  network.start()
  control.start()

  for thread in threading.enumerate():
    if thread is not threading.currentThread():
      thread.join()
