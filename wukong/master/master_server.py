#!/usr/bin/python
# vim: ts=2 sw=2 expandtab

# author: Penn Su
from gevent import monkey; monkey.patch_all()
import gevent
import serial
import platform
import os, sys, zipfile, re, time
import tornado.ioloop, tornado.web
import tornado.template as template
import simplejson as json
from jinja2 import Template
import logging
import hashlib
from threading import Thread
import traceback
import StringIO
import shutil, errno
import datetime
import glob
import copy
import fcntl, termios, struct

import tornado.options
tornado.options.define("appdir", type=str, help="Directory that contains the applications")
tornado.options.parse_command_line()
from configuration import *

if WKPFCOMM_AGENT == "ZWAVE":
  try:
    import pyzwave
    m = pyzwave.getDeviceType
  except:
    print "Please install the pyzwave module in the wukong/tools/python/pyzwave by using"
    print "cd ../tools/python/pyzwave; sudo python setup.py install"
    sys.exit(-1)
import wkpf.wusignal
from wkpf.wuapplication import WuApplication
from wkpf.wuclasslibraryparser import *
from wkpf.wkpfcomm import *
from wkpf.util import *

import wkpf.globals
from configuration import *

import tornado.options

if(MONITORING == 'true'):
    try:
      from pymongo import MongoClient
    except:
      print "Please install python mongoDB driver pymongo by using"
      print "easy_install pymongo"
      sys.exit(-1)

    try:
        wkpf.globals.mongoDBClient = MongoClient(MONGODB_URL)

    except:
      print "MongoDB instance " + MONGODB_URL + " can't be connected."
      print "Please install the mongDB, pymongo module."
      sys.exit(-1)

tornado.options.parse_command_line()
#tornado.options.enable_pretty_logging()

IP = sys.argv[1] if len(sys.argv) >= 2 else '127.0.0.1'

landId = 100
node_infos = []



from make_js import make_main
from make_fbp import fbp_main
def import_wuXML():
	make_main()
	
def make_FBP():
	test_1 = fbp_main()
	test_1.make()	

wkpf.globals.location_tree = LocationTree(LOCATION_ROOT)

def initializeVirtualNode():
    # Add the server as a virtual Wudevice for monitoring
    wuclasses = {}
    wuobjects = {}

    # 1 is by default the network id of the controller
    node = WuNode(1, '/' + LOCATION_ROOT, wuclasses, wuobjects, 'virtualdevice')
    wuclassdef = WuObjectFactory.wuclassdefsbyid[44]
    wuobject = WuObjectFactory.createWuObject(wuclassdef, node, 1, False)
    wkpf.globals.virtual_nodes[1] = node


# using cloned nodes
def rebuildTree(nodes):
  nodes_clone = copy.deepcopy(nodes)
  wkpf.globals.location_tree = LocationTree(LOCATION_ROOT)
  wkpf.globals.location_tree.buildTree(nodes_clone)
  flag = os.path.exists("../LocalData/landmarks.txt")
  if(flag):
      wkpf.globals.location_tree.loadTree()
  wkpf.globals.location_tree.printTree()

# Helper functions
def setup_signal_handler_greenlet():
  logging.info('setting up signal handler')
  gevent.spawn(wusignal.signal_handler)

def getAppIndex(app_id):
  # make sure it is not unicode
  app_id = app_id.encode('ascii','ignore')
  for index, app in enumerate(wkpf.globals.applications):
    if app.id == app_id:
      return index
  return None

def delete_application(i):
  try:
    shutil.rmtree(wkpf.globals.applications[i].dir)
    wkpf.globals.applications.pop(i)
    return True
  except Exception as e:
    exc_type, exc_value, exc_traceback = sys.exc_info()
    print traceback.print_exception(exc_type, exc_value, exc_traceback,
                                  limit=2, file=sys.stdout)
    return False

def load_app_from_dir(dir):
  app = WuApplication(dir=dir)
  app.loadConfig()
  return app

def update_applications():
  logging.info('updating applications:')

  application_basenames = [os.path.basename(app.dir) for app in wkpf.globals.applications]

  for dirname in os.listdir(APP_DIR):
    app_dir = os.path.join(APP_DIR, dirname)
    if dirname.lower() == 'base': continue
    if not os.path.isdir(app_dir): continue

    logging.info('scanning %s:' % (dirname))
    if dirname not in application_basenames:
      logging.info('%s' % (dirname))
      wkpf.globals.applications.append(load_app_from_dir(app_dir))
      application_basenames = [os.path.basename(app.dir) for app in wkpf.globals.applications]

class idemain(tornado.web.RequestHandler):
  def get(self):
    self.content_type='text/html'
    self.render('templates/ide.html')
# List all uploaded applications
class main(tornado.web.RequestHandler):
  def get(self):
    getComm()
    self.render('templates/application.html', connected=wkpf.globals.connected)

class list_applications(tornado.web.RequestHandler):
  def get(self):
    self.render('templates/index.html', applications=wkpf.globals.applications)

  def post(self):
    update_applications()
    apps = sorted([application.config() for application in wkpf.globals.applications], key=lambda k: k['app_name'])
    self.content_type = 'application/json'
    self.write(json.dumps(apps))

# Returns a form to upload new application
class new_application(tornado.web.RequestHandler):
  def post(self):
    #self.redirect('/applications/'+str(applications[-1].id), permanent=True)
    #self.render('templates/upload.html')
    try:
      try:
        app_name = self.get_argument('app_name')
      except:
        app_name = 'application' + str(len(wkpf.globals.applications))
      app_id = hashlib.md5(app_name).hexdigest()

      if getAppIndex(app_id):
        self.content_type = 'application/json'
        self.write({'status':1, 'mesg':'Cannot create application with the same name'})
        return

      # copy base for the new application
      logging.info('creating application... "%s"' % (app_name))
      copyAnything(BASE_DIR, os.path.join(APP_DIR, app_id))

      app = WuApplication(id=app_id, app_name=app_name, dir=os.path.join(APP_DIR, app_id))
      logging.info('app constructor')
      logging.info(app.app_name)

      wkpf.globals.applications.append(app)

      # dump config file to app
      logging.info('saving application configuration...')
      app.saveConfig()

      self.content_type = 'application/json'
      self.write({'status':0, 'app': app.config()})
    except Exception as e:
      exc_type, exc_value, exc_traceback = sys.exc_info()
      print traceback.print_exception(exc_type, exc_value, exc_traceback,
                                  limit=2, file=sys.stdout)
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg':'Cannot create application'})

class rename_application(tornado.web.RequestHandler):
  def put(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      try:
        wkpf.globals.applications[app_ind].app_name = self.get_argument('value', '')
        wkpf.globals.applications[app_ind].saveConfig()
        self.content_type = 'application/json'
        self.write({'status':0})
      except Exception as e:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        print traceback.print_exception(exc_type, exc_value, exc_traceback,
                                      limit=2, file=sys.stdout)
        self.set_status(400)
        self.content_type = 'application/json'
        self.write({'status':1, 'mesg': 'Cannot save application'})

class application(tornado.web.RequestHandler):
  # topbar info
  def get(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      title = ""
      if self.get_argument('title'):
        title = self.get_argument('title')
      app = wkpf.globals.applications[app_ind].config()
      topbar = template.Loader(os.getcwd()).load('templates/topbar.html').generate(application=wkpf.globals.applications[app_ind], title=title, default_location=LOCATION_ROOT)
      self.content_type = 'application/json'
      self.write({'status':0, 'app': app, 'topbar': topbar})

  # Display a specific application
  def post(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      # active application
      wkpf.globals.set_active_application_index(app_ind)
      app = wkpf.globals.applications[app_ind].config()
      topbar = template.Loader(os.getcwd()).load('templates/topbar.html').generate(application=wkpf.globals.applications[app_ind], title="Flow Based Programming")
      self.content_type = 'application/json'
      self.write({'status':0, 'app': app, 'topbar': topbar})

  # Update a specific application
  def put(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      try:
        wkpf.globals.applications[app_ind].app_name = self.get_argument('name', '')
        wkpf.globals.applications[app_ind].desc = self.get_argument('desc', '')
        wkpf.globals.applications[app_ind].saveConfig()
        self.content_type = 'application/json'
        self.write({'status':0})
      except Exception as e:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        print traceback.print_exception(exc_type, exc_value, exc_traceback,
                                      limit=2, file=sys.stdout)
        self.content_type = 'application/json'
        self.write({'status':1, 'mesg': 'Cannot save application'})

  # Destroy a specific application
  def delete(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      if delete_application(app_ind):
        self.content_type = 'application/json'
        self.write({'status':0})
      else:
        self.content_type = 'application/json'
        self.write({'status':1, 'mesg': 'Cannot delete application'})

class reset_application(tornado.web.RequestHandler):
  def post(self, app_id):
    app_ind = getAppIndex(app_id)

    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      wkpf.globals.set_wukong_status("close")
      wkpf.globals.applications[app_ind].status = "close"
      self.content_type = 'application/json'
      self.write({'status':0, 'version': wkpf.globals.applications[app_ind].version})

class deploy_application(tornado.web.RequestHandler):
  def get(self, app_id):
    global node_infos
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      # deployment.js will call refresh_node eventually, rebuild location tree there
      deployment = template.Loader(os.getcwd()).load('templates/deployment.html').generate(
              app=wkpf.globals.applications[app_ind],
              app_id=app_id, node_infos=node_infos,
              logs=wkpf.globals.applications[app_ind].logs(),
              changesets=wkpf.globals.applications[app_ind].changesets, 
              set_location=False, 
              default_location=LOCATION_ROOT)
      self.content_type = 'application/json'
      self.write({'status':0, 'page': deployment})

  def post(self, app_id):
    app_ind = getAppIndex(app_id)

    wkpf.globals.set_wukong_status("Deploying")
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      platforms = ['avr_mega2560']
      # signal deploy in other greenlet task
      wusignal.signal_deploy(platforms)
      wkpf.globals.set_active_application_index(app_ind)
         
      self.content_type = 'application/json'
      self.write({
        'status':0,
        'version': wkpf.globals.applications[app_ind].version})

class map_application(tornado.web.RequestHandler):
  def post(self, app_id):

    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      platforms = ['avr_mega2560']
      # TODO: need platforms from fbp
      #node_infos = getComm().getActiveNodeInfos()
      rebuildTree(node_infos)

      # Map with location tree info (discovery), this will produce mapping_results
      #mapping_result = wkpf.globals.applications[app_ind].map(wkpf.globals.location_tree, getComm().getRoutingInformation())
      mapping_result = wkpf.globals.applications[app_ind].map(wkpf.globals.location_tree, [])

      ret = []
      for component in wkpf.globals.applications[app_ind].changesets.components:
        obj_hash = {
          'instanceId': component.index,
          'location': component.location,
          'group_size': component.group_size,
          'name': component.type,
          'msg' : component.message,
          'instances': []
        }

        for wuobj in component.instances:
          wuobj_hash = {
            'instanceId': component.index,
            'name': component.type,
            'nodeId': wuobj.wunode.id,
            'portNumber': wuobj.port_number,
            'virtual': wuobj.virtual
          }

          obj_hash['instances'].append(wuobj_hash)
        
        ret.append(obj_hash)

      self.content_type = 'application/json'
      self.write({
        'status':0,
        'mapping_result': mapping_result, # True or False
        'mapping_results': ret,
        'version': wkpf.globals.applications[app_ind].version,
        'mapping_status': wkpf.globals.applications[app_ind].mapping_status})

class monitor_application(tornado.web.RequestHandler):
  def get(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    #elif not applications[app_ind].mapping_results or not applications[app_ind].deployed:
      #self.content_type = 'application/json'
      #self.wrtie({'status':1, 'mesg': 'No mapping results or application out of sync, please deploy the application first.'})
    else:

      properties_json = WuProperty.all() # for now
      #properties_json = getPropertyValuesOfApp(applications[app_ind].mapping_results, [property.getName() for wuobject in applications[app_ind].mapping_results.values() for property in wuobject])

      monitor = template.Loader(os.getcwd()).load('templates/monitor.html').generate(app=wkpf.globals.applications[app_ind], logs=wkpf.globals.applications[app_ind].logs(), properties_json=properties_json)
      self.content_type = 'application/json'
      self.write({'status':0, 'page': monitor})

class properties_application(tornado.web.RequestHandler):
  def post(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      properties_json = WuProperty.all() # for now
      #properties_json = getPropertyValuesOfApp(applications[app_ind].mapping_results, [property.getName() for wuobject in applications[app_ind].mapping_results.values() for property in wuobject])

      self.content_type = 'application/json'
      self.write({'status':0, 'properties': properties_json})

# Never let go
class poll(tornado.web.RequestHandler):
  def post(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      application = wkpf.globals.applications[app_ind]

      self.content_type = 'application/json'
      self.write({
        'status':0,
        'ops': application.deploy_ops,
        'version': application.version,
        'deploy_status': application.deploy_status,
        'mapping_status': application.mapping_status,
        'wukong_status': wkpf.globals.get_wukong_status(),
        'application_status': application.status, 
        'returnCode': application.returnCode})

      # TODO: log should not be requested in polling, should be in a separate page
      # dedicated for it
      # because logs could go up to 10k+ entries
      #'logs': wkpf.globals.applications[app_ind].logs()

class save_fbp(tornado.web.RequestHandler):
  def post(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      xml = self.get_argument('xml')
      wkpf.globals.applications[app_ind].updateXML(xml)
      #applications[app_ind] = load_app_from_dir(applications[app_ind].dir)
      #applications[app_ind].xml = xml
      # TODO: need platforms from fbp
      #platforms = self.get_argument('platforms')
      platforms = ['avr_mega2560']

      self.content_type = 'application/json'
      self.write({'status':0, 'version': wkpf.globals.applications[app_ind].version})

class load_fbp(tornado.web.RequestHandler):
  def get(self, app_id):
    self.render('templates/fbp.html')

  def post(self, app_id):
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      self.content_type = 'application/json'
      self.write({'status':0, 'xml': wkpf.globals.applications[app_ind].xml})

class poll_testrtt(tornado.web.RequestHandler):
  def post(self):
    comm = getComm()
    status = comm.currentStatus()
    if status != None:
      self.content_type = 'application/json'
      self.write({'status':0, 'logs': status.split('\n')})
    else:
      self.content_type = 'application/json'
      self.write({'status':0, 'logs': []})

class stop_testrtt(tornado.web.RequestHandler):
  def post(self):
    comm = getComm()
    node_infos = comm.updateAllNodeInfos()
    rebuildTree(node_infos)
    if comm.onStopMode():
      self.content_type = 'application/json'
      self.write({'status':0})
    else:
      self.content_type = 'application/json'
      self.write({'status':1})

class exclude_testrtt(tornado.web.RequestHandler):
  def post(self):
    comm = getComm()
    if comm.onDeleteMode():
      self.content_type = 'application/json'
      self.write({'status':0, 'log': 'Going into exclude mode'})
    else:
      self.content_type = 'application/json'
      self.write({'status':1, 'log': 'There is an error going into exclude mode'})

class include_testrtt(tornado.web.RequestHandler):
  def post(self):
    comm = getComm()
    if comm.onAddMode():
      self.content_type = 'application/json'
      self.write({'status':0, 'log': 'Going into include mode'})
    else:
      self.content_type = 'application/json'
      self.write({'status':1, 'log': 'There is an error going into include mode'})

class testrtt(tornado.web.RequestHandler):
  def get(self):
    global node_infos

    comm = getComm()
    node_infos = comm.getAllNodeInfos(False)
    
    rebuildTree(node_infos)
    testrtt = template.Loader(os.getcwd()).load('templates/testrtt.html').generate(log=['Please press the buttons to add/remove nodes.'], node_infos=node_infos, set_location=True, default_location = LOCATION_ROOT)
    self.content_type = 'application/json'
    self.write({'status':0, 'testrtt':testrtt})

class refresh_nodes(tornado.web.RequestHandler):
  def post(self, force):
    global node_infos

    if int(force,0) == 0:
      node_infos = getComm().getActiveNodeInfos(False)
    else:
      node_infos = getComm().getActiveNodeInfos(True)
    rebuildTree(node_infos)
    print ("node_infos in refresh nodes:",node_infos)
    #furniture data loaded from fake data for purpose of 
    #getComm().getRoutingInformation()
    # default is false
    set_location = self.get_argument('set_location', False)
    if set_location == u'True':
      set_location = True
    else:
      set_location = False

    nodes = template.Loader(os.getcwd()).load('templates/monitor-nodes.html').generate(node_infos=node_infos, set_location=set_location, default_location=LOCATION_ROOT)

    self.content_type = 'application/json'
    self.write({'status':0, 'nodes': nodes})

class nodes(tornado.web.RequestHandler):
  def get(self):
    pass

  def post(self, nodeId):
    info = None
    comm = getComm()
    info = comm.getNodeInfo(nodeId)
    
    self.content_type = 'application/json'
    self.write({'status':0, 'node_info': info})

  def put(self, nodeId):
    global node_infos
    location = self.get_argument('location')
    print node_infos
    print 'in nodes: simulation:'+SIMULATION
    if SIMULATION == "true":
      for info in node_infos:
        if info.id == int(nodeId):
          info.location = location
          senNd = SensorNode(info)
          WuNode.saveNodes()
          wkpf.globals.location_tree.addSensor(senNd)
      wkpf.globals.location_tree.printTree()
      self.content_type = 'application/json'
      self.write({'status':0})
      return
    comm = getComm()
    if location:
       #print "nodeId=",nodeId
       info = comm.getNodeInfo(int(nodeId))
       #print "device type=",info.type
       if info.type == 'native':
         info.location = location
         WuNode.saveNodes()
         senNd = SensorNode(info)
         wkpf.globals.location_tree.addSensor(senNd)
         #wkpf.globals.location_tree.printTree()
         self.content_type = 'application/json'
         self.write({'status':0})
       else:
         if comm.setLocation(int(nodeId), location):
          # update our knowledge too
            for info in comm.getActiveNodeInfos():
              if info.id == int(nodeId):
                info.location = location
                senNd = SensorNode(info)
                print (info.location)
            wkpf.globals.location_tree.addSensor(senNd)
            wkpf.globals.location_tree.printTree()
            WuNode.saveNodes()
            self.content_type = 'application/json'
            self.write({'status':0})
         else:
            self.content_type = 'application/json'
            self.write({'status':1, 'mesg': 'Cannot set location, please try again.'})

class WuLibrary(tornado.web.RequestHandler):	
  def get(self):
    self.content_type = 'application/xml'
    try:
      f = open('../ComponentDefinitions/WuKongStandardLibrary.xml')
      xml = f.read()
      f.close()
    except:
      self.write('<error>1</error>')
    self.write(xml)
  def post(self):
    xml = self.get_argument('xml')
    try:
      f = open('../ComponentDefinitions/WuKongStandardLibrary.xml','w')
      xml = f.write(xml)
      f.close()
    except:
      self.write('<error>1</error>')
    self.write('')
class WuLibraryUser(tornado.web.RequestHandler):	
  def get(self):
    self.content_type = 'application/xml'
    appid = self.get_argument('appid')
    app = wkpf.globals.applications[getAppIndex(appid)]
    print app.dir
    try:
      f = open(app.dir+'/WKDeployCustomComponents.xml')
      xml = f.read()
      f.close()
      self.write(xml)
    except:
      self.write('<WuKong><WuClass name="Custom1" id="100"></WuClass></WuKong>')
      return
  def post(self):
    xml = self.get_argument('xml')
    appid = self.get_argument('appid')
    app = wkpf.globals.applications[getAppIndex(appid)]
    try:
      component_path = app.dir+'/WKDeployCustomComponents.xml'
      f = open(component_path, 'w')
      xml = f.write(xml)
      f.close()
      make_main(component_path)
    except:
      self.write('<error>1</error>')
    self.write('')

class SerialPort(tornado.web.RequestHandler):
  def get(self):
    self.content_type = 'application/json'
    system_name = platform.system()
    if system_name == "Windows":
      available = []
      for i in range(256):
        try:
          s = serial.Serial(i)
          available.append(i)
          s.close()
        except:
          pass
      self.write(json.dumps(available))
      return
    if system_name == "Darwin":
      list = glob.glob('/dev/tty.*') + glob.glob('/dev/cu.*')
    else:
      print 'xxxxx'
      list = glob.glob('/dev/ttyS*') + glob.glob('/dev/ttyUSB*') + glob.glob('/dev/ttyACM*')
    available=[]
    for l in list:
      try:
        s = serial.Serial(l)
        available.append(l)
        s.close()
      except:
        pass
    self.write(json.dumps(available))

class EnabledWuClass(tornado.web.RequestHandler):	
  def get(self):
    self.content_type = 'application/xml'
    try:
      f = open('../../src/config/wunode/enabled_wuclasses.xml')
      xml = f.read()
      f.close()
    except:
      self.write('<error>1</error>')
    self.write(xml)
  def post(self):
    try:
      f = open('../../src/config/wunode/enabled_wuclasses.xml','w')
      xml = self.get_argument('xml')
      f.write(xml)
      f.close()
    except:
      pass

class WuClassSource(tornado.web.RequestHandler):	
  def get(self):
    self.content_type = 'text/plain'
    try:
      name = self.get_argument('src')
      type = self.get_argument('type')
      appid = self.get_argument('appid', None)
      app = None
      if appid:
          app = wkpf.globals.applications[getAppIndex(appid)]

      if type == 'C':
        name_ext = 'wuclass_'+Convert.to_c(name)+'_update.c'
      else:
        name_ext = 'Virtual'+Convert.to_java(name)+'WuObject.java'
      try:
          f = open(self.findPath(name_ext, app))
          cont = f.read()
          f.close()
      except:
        traceback.print_exc()
        # We may use jinja2 here
        if type == "C":
          f = open('templates/wuclass.tmpl.c')
          classname = Convert.to_c(name)
        else:
          f = open('templates/wuclass.tmpl.java')
          classname = Convert.to_java(name)

        template = Template(f.read())
        f.close()
        cont = template.render(classname=classname)
    except:
      self.write(traceback.format_exc())
      return
    self.write(cont)
  def post(self):
    try:
      print 'xxx'
      name = self.get_argument('name')
      type = self.get_argument('type')
      appid = self.get_argument('appid', None)
      app = None
      if appid:
          app = wkpf.globals.applications[getAppIndex(appid)]

      if type == 'C':
        name_ext = 'wuclass_'+Convert.to_c(name)+'_update.c'
      else:
        name_ext = 'Virtual'+Convert.to_java(name)+'WuObject.java'
      try:
        f = open(self.findPath(name_ext, app), 'w')
      except:
        traceback.print_exc()
        if type == 'C':
          f = open("../../src/lib/wkpf/c/common/native_wuclasses/"+name_ext,'w')
        else:
          f = open("../javax/wukong/virtualwuclasses/"+name_ext,'w')
      f.write(self.get_argument('content'))
      f.close()
      self.write('OK')
    except:
      self.write('Error')
      print traceback.format_exc()

  def findPath(self, p, app=None):
    # Precedence of path is All apps dir -> common native wuclasses, then arcuino native wuclasses
    paths = [os.path.join(APP_DIR, dirname) for dirname in os.listdir(APP_DIR)] + ['../../src/lib/wkpf/c/common/native_wuclasses/', '../../src/lib/wkpf/c/arduino/native_wuclasses/','../javax/wukong/virtualwuclasses/']
    # If an app is passed in, then its dir will be the first to search
    if app:
      paths = [app.dir] 
    for path in paths:
      if not os.path.isdir(path): continue
      filename = path +'/'+ p
      print filename
      if os.path.isfile(filename):
        return filename
    # returns None if not found
    return None

class loc_tree(tornado.web.RequestHandler): 
  def post(self):
    global node_infos
      
    addloc = template.Loader(os.getcwd()).load('templates/display_locationTree.html').generate(node_infos=node_infos)
    wkpf.globals.location_tree.printTree()
    disploc = wkpf.globals.location_tree.getJson()

    self.content_type = 'application/json'
    self.write({'loc':json.dumps(disploc),'node':addloc})
  
  def get(self, node_id):
    global node_infos
    node_id = int(node_id)
    curNode = wkpf.globals.location_tree.findLocationById(node_id)
    print node_id, curNode
    if curNode == None:
        self.write({'status':1,'message':'cannot find node id '+str(node_id)})
        return
    else:
        print curNode.distanceModifier
        self.write({'status':0, 'message':'succeed in finding node id'+str(node_id), 
                    'distanceModifierByName':curNode.distanceModifierToString(), 'distanceModifierById':curNode.distanceModifierIdToString(),
                    'centerPnt':curNode.centerPnt, 
                    'size':curNode.size, 'location':curNode.getLocationStr(), 'local_coord':curNode.getOriginalPnt(),
                    'global_coord':curNode.getGlobalOrigPnt(), 'landmarks': json.dumps(curNode.getLandmarkList())}) 
    
  def put(self, node_id):
    global node_infos
    node_id = int(node_id)
    curNode = wkpf.globals.location_tree.findLocationById(node_id)
    if curNode == None:
        self.write({'status':1,'message':'cannot find node id '+str(node_id)})
        return
    else:
        global_coord = self.get_argument("global_coord")
        local_coord = self.get_argument("local_coord")
        size = self.get_argument("size")
        direction = self.get_argument("direction")
        curNode.setLGSDFromStr(local_coord, global_coord, size, direction)
        self.write({'status':0,'message':'find node id '+str(node_id)})

class sensor_info(tornado.web.RequestHandler):  
    def get(self, node_id, sensor_id):
        global node_infos
        node_id = int(node_id)
        curNode = wkpf.globals.location_tree.findLocationById(node_id)
        if curNode == None:
            self.write({'status':1,'message':'cannot find node id '+str(node_id)})
            return
        if sensor_id[0:2] =='se':   #sensor case
            se_id = int(sensor_id[2:])
            sensr = curNode.getSensorById(se_id)
            self.write({'status':0,'message':'find sensor id '+str(se_id), 'location':sensr.location})
        elif sensor_id[0:2] =='lm': #landmark case
            lm_id = int(sensor_id[2:])
            landmk = curNode.findLandmarkById(lm_id)
            self.write({'status':0,'message':'find landmark id '+str(lm_id), 'location':landmk.location,'size':landmk.size, 'direction':landmk.direction})
        else:
            self.write({'status':1, 'message':'failed in finding '+sensor_id+" in node"+ str(node_id)})

class edit_loc_tree(tornado.web.RequestHandler):
    def post(self):
        operation = self.get_argument("operation")
        parent_id = self.get_argument("parent_id")
        child_name = self.get_argument("child_name")
        size = self.get_argument("size")
        paNode = wkpf.globals.location_tree.findLocationById(int(parent_id))
        if paNode != None:
            if operation == "0": #add a new location
                paNode.addChild(child_name)
                print ("add child", child_name)
                self.write({'status':0,'message':'successfully add child '+child_name })
                return
            elif operation == "1": #delete a location
                childNode = paNode.findChildByName(child_name)
                if childNode != None:
                    paNode.delChild(childNode)
                    self.write({'status':0,'message':'successfully delete child '+child_name })
                    return
                else:
                    self.write({'status':1,'message': child_name +'not found' })
                    return
        else:
            self.write({'status':1,'message':'parentNode does not exist in location tree :('})
            return
                

class tree_modifier(tornado.web.RequestHandler):
  def put(self, mode):
    start_id = self.get_argument("start")
    end_id = self.get_argument("end")
    distance = self.get_argument("distance")
    paNode = wkpf.globals.location_tree.findLocationById(int(start_id)//100)      #find parent node
    if paNode !=None:
        if int(mode) == 0:        #adding modifier between siblings
            if paNode.addDistanceModifier(int(start_id), int(end_id), int(distance)):
                self.write({'status':0,'message':'adding distance modifier between '+str(start_id) +'and'+str(end_id)+'to node'+str(int(start_id)//100)})
                return
            else:
                self.write({'status':1,'message':'adding failed due to not able to find common direct father of the two nodes'})
                return
        elif int(mode) == 1:        #deleting modifier between siblings
            if paNode.delDistanceModifier(int(start_id), int(end_id), int(distance)):
                self.write({'status':0,'message':'deletinging distance modifier between '+str(start_id) +'and'+str(end_id)+'to node'+str(int(start_id)//100)})
                return
            else:
                self.write({'status':1,'message':'deleting faild due to not able to find common direct father of the two nodes'})
                return
    self.write({'status':1,'message':'operation faild due to not able to find common direct father of the two nodes'})
    
 
class save_landmark(tornado.web.RequestHandler):
  def put(self):
        
        self.write({'tree':wkpf.globals.location_tree})

  def post(self):
        wkpf.globals.location_tree.saveTree()
        self.write({'message':'Save Successfully!'})
        
class load_landmark(tornado.web.RequestHandler):
    def post(self):
        flag = os.path.exists("../LocalData/landmarks.txt")
        if(flag):
            wkpf.globals.location_tree.loadTree()
            self.write({'message':'Load Successfully!'})
        else:
            self.write({'message':'"../LocalData/landmarks.txt" does not exist '})
        
class add_landmark(tornado.web.RequestHandler):
  def put(self):
    global landId

    name = self.get_argument("name")
    id = 0;
    location = self.get_argument("location")
    operation = self.get_argument("ope")
    size  = self.get_argument("size")
    direct = self.get_argument("direction")
    coordinate = self.get_argument("coordinate")
    landmark = None
    rt_val = 0
    msg = ''
    if(operation=="1"):
      landId += 1
      landmark = LandmarkNode(name, location+"@"+coordinate, size, direct) 
      rt_val = wkpf.globals.location_tree.addLandmark(landmark)
      msg = 'add fails'
      wkpf.globals.location_tree.printTree()
    elif(operation=="0"):
      rt_val = wkpf.globals.location_tree.delLandmark(name, location)
      msg = 'deletion of '+ name + ' fails at '+ location
    self.content_type = 'application/json'
    if rt_val == True:
        self.write({'status':0, 'id':name, 'msg':'change succeeds'})
    if rt_val == False:
        self.write({'status':1, 'id':name, 'msg':msg})

class Build(tornado.web.RequestHandler):  
  def get(self):
    self.content_type = 'text/plain'
    cmd = self.get_argument('cmd')
    if cmd == 'start':
      command = 'cd ../../src/config/wunode; rm -f tmp'
      os.system(command)
      os.system('(cd ../../src/config/wunode; ant 2>&1 | cat > tmp)&')
      log = 'start'
    elif cmd == 'poll':
      f = open("../../src/config/wunode/tmp", "r")
      log = f.readlines()
      log = "".join(log)
      f.close()

    self.write(log)


class Upload(tornado.web.RequestHandler):  
  def get(self):
    self.content_type = 'text/plain'
    cmd = self.get_argument('cmd')
    if cmd == 'start':
      port = self.get_argument("port")
      command = 'cd ../../src/config/wunode; rm -f tmp'
      os.system(command)
      f = open("../../src/settings.xml","w")
      s = '<project name="settings">' + '\n' + \
        '\t<property name="avrdude-programmer" value="' + port + '"/>' + '\n' + \
        '</project>'
      f.write(s)
      f.close()
      s = open(port)
      dtr = struct.pack('I', termios.TIOCM_DTR)
      fcntl.ioctl(s, termios.TIOCMBIS, dtr)
      fcntl.ioctl(s, termios.TIOCMBIC, dtr)
      s.close()
      
      command = 'killall avrdude;(cd ../../src/config/wunode; ant avrdude 2>&1 | cat> tmp)&'
      os.system(command)
      log='start'
    elif cmd == 'poll':
      f = open("../../src/config/wunode/tmp", "r")
      log = f.readlines()
      log = "".join(log)
      f.close()


    #p = sub.Popen(command, stdout=sub.PIPE, stderr=sub.PIPE)
    #output, errors = p.communicate()
    #f = open("../../src/config/wunode/j", "w")
    #f.write(output)
    #f.close()

    self.write(log)



settings = dict(
  static_path=os.path.join(os.path.dirname(__file__), "static"),
  debug=True
)

ioloop = tornado.ioloop.IOLoop.instance()
wukong = tornado.web.Application([
  (r"/", main),
  (r"/ide", idemain),
  (r"/main", main),
  (r"/testrtt/exclude", exclude_testrtt),
  (r"/testrtt/include", include_testrtt),
  (r"/testrtt/stop", stop_testrtt),
  (r"/testrtt/poll", poll_testrtt),
  (r"/testrtt", testrtt),
  (r"/nodes/([0-9]*)", nodes),
  (r"/nodes/refresh/([0-9])", refresh_nodes),
  (r"/applications", list_applications),
  (r"/applications/new", new_application),
  (r"/applications/([a-fA-F\d]{32})", application),
  (r"/applications/([a-fA-F\d]{32})/rename", rename_application),
  (r"/applications/([a-fA-F\d]{32})/reset", reset_application),
  (r"/applications/([a-fA-F\d]{32})/properties", properties_application),
  (r"/applications/([a-fA-F\d]{32})/poll", poll),
  (r"/applications/([a-fA-F\d]{32})/deploy", deploy_application),
  (r"/applications/([a-fA-F\d]{32})/deploy/map", map_application),
  (r"/applications/([a-fA-F\d]{32})/monitor", monitor_application),
  (r"/applications/([a-fA-F\d]{32})/fbp/save", save_fbp),
  (r"/applications/([a-fA-F\d]{32})/fbp/load", load_fbp),
  (r"/loc_tree/nodes/([0-9]*)", loc_tree),
  (r"/loc_tree/edit", edit_loc_tree),
  (r"/loc_tree/nodes/([0-9]*)/(\w+)", sensor_info),
  (r"/loc_tree", loc_tree),
  (r"/loc_tree/modifier/([0-9]*)", tree_modifier),
  (r"/loc_tree/save", save_landmark),
  (r"/loc_tree/load", load_landmark),
  (r"/loc_tree/land_mark", add_landmark),
  (r"/componentxml",WuLibrary),
  (r"/componentxmluser",WuLibraryUser),
  (r"/wuclasssource",WuClassSource),
  (r"/serialport",SerialPort),
  (r"/enablexml",EnabledWuClass),
  (r"/build",Build),
  (r"/upload",Upload)
], IP, **settings)

logging.info("Starting up...")
setup_signal_handler_greenlet()
WuClassLibraryParser.read(COMPONENTXML_PATH)
initializeVirtualNode();
#WuNode.loadNodes()
update_applications()
import_wuXML()
make_FBP()
wukong.listen(MASTER_PORT)

if __name__ == "__main__":
  ioloop.start()
