#!/usr/bin/python
# author: Penn Su
from gevent import monkey; monkey.patch_all()
import gevent
import serial
import platform
import os, sys, zipfile, re, time
import tornado.ioloop, tornado.web
import tornado.template as template
import simplejson as json
import logging
import hashlib
from threading import Thread
import traceback
import StringIO
import shutil, errno
import datetime
import glob

import wkpf.wusignal
from wkpf.wuapplication import WuApplication
from wkpf.parser import *
from wkpf.wkpfcomm import *

from wkpf.globals import *
from configuration import *

import tornado.options
tornado.options.parse_command_line()
tornado.options.enable_pretty_logging()

IP = sys.argv[1] if len(sys.argv) >= 2 else '127.0.0.1'

landId = 100
node_infos = []

from make_js import make_main
from make_fbp import fbp_main
def import_wuXML():
	test = make_main()
	test.make()
	
def make_FBP():
	test_1 = fbp_main()
	test_1.make()	

location_tree = LocationTree(LOCATION_ROOT)
routingTable = {}

# Helper functions
def setup_signal_handler_greenlet():
  logging.info('setting up signal handler')
  gevent.spawn(wusignal.signal_handler)
def allowed_file(filename):
  return '.' in filename and \
      filename.rsplit('.', 1)[1] in ALLOWED_EXTENSIONS

def copyAnything(src, dst):
  try:
    shutil.copytree(src, dst)
  except OSError as exc: # python >2.5
    exc_type, exc_value, exc_traceback = sys.exc_info()
    print traceback.print_exception(exc_type, exc_value, exc_traceback,
                                  limit=2, file=sys.stdout)
    if exc.errno == errno.ENOTDIR:
      shutil.copy(src, dst)
    else: raise

def getAppIndex(app_id):
  global applications
  # make sure it is not unicode
  app_id = app_id.encode('ascii','ignore')
  for index, app in enumerate(applications):
    if app.id == app_id:
      return index
  return None

def delete_application(i):
  global applications
  try:
    shutil.rmtree(applications[i].dir)
    #os.system('rm -rf ' + applications[i].dir)
    applications.pop(i)
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
  global applications
  logging.info('updating applications:')

  application_basenames = [os.path.basename(app.dir) for app in applications]

  for dirname in os.listdir(APP_DIR):
    app_dir = os.path.join(APP_DIR, dirname)
    if dirname.lower() == 'base': continue
    if not os.path.isdir(app_dir): continue

    logging.info('scanning %s:' % (dirname))
    if dirname not in application_basenames:
      logging.info('%s' % (dirname))
      applications.append(load_app_from_dir(app_dir))
      application_basenames = [os.path.basename(app.dir) for app in applications]

# deprecated
def getPropertyValuesOfApp(mapping_results, property_names):
  properties_json = []

  comm = getComm()
  for wuobject in mapping_results.values():
    for name in property_names:
      if name in wuobject:
        wuproperty = wuobject.getPropertyByName(name)
        (value, datatype, status) = comm.getProperty(wuobject, int(wuproperty.getId()))
        properties_json.append({'name': name, 'value': value, 'wuclassname': wuproperty.wuclass.name})

  return properties_json

class idemain(tornado.web.RequestHandler):
  def get(self):
    self.content_type='text/html'
    self.render('templates/ide.html')
# List all uploaded applications
class main(tornado.web.RequestHandler):
  def get(self):
    self.render('templates/application.html')

class list_applications(tornado.web.RequestHandler):
  def get(self):
    self.render('templates/index.html', applications=applications)

  def post(self):
    global applications
    update_applications()
    apps = sorted([application.config() for application in applications], key=lambda k: k['name'])
    self.content_type = 'application/json'
    self.write(json.dumps(apps))

# Returns a form to upload new application
class new_application(tornado.web.RequestHandler):
  def post(self):
    global applications
    #self.redirect('/applications/'+str(applications[-1].id), permanent=True)
    #self.render('templates/upload.html')
    try:
      app_name = 'application' + str(len(applications))
      app_id = hashlib.md5(app_name).hexdigest()

      # copy base for the new application
      logging.info('creating application...')
      copyAnything(BASE_DIR, os.path.join(APP_DIR, app_id))

      app = WuApplication(id=app_id, name=app_name, dir=os.path.join(APP_DIR, app_id))
      applications.append(app)

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
      app = applications[app_ind].config()
      topbar = template.Loader(os.getcwd()).load('templates/topbar.html').generate(application=applications[app_ind], title=title, default_location=LOCATION_ROOT)
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
      set_active_application_index(app_ind)
      app = applications[app_ind].config()
      #app = {'name': applications[app_ind].name, 'desc': applications[app_ind].desc, 'id': applications[app_ind].id}
      topbar = template.Loader(os.getcwd()).load('templates/topbar.html').generate(application=applications[app_ind], title="Flow Based Programming")
      self.content_type = 'application/json'
      self.write({'status':0, 'app': app, 'topbar': topbar})

  # Update a specific application
  def put(self, app_id):
    global applications
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      try:
        applications[app_ind].name = self.get_argument('name', 'application name')
        applications[app_ind].desc = self.get_argument('desc', '')
        applications[app_ind].saveConfig()
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
    global applications
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
      set_wukong_status("clear")
      applications[app_ind].status = "clear"
      self.content_type = 'application/json'
      self.write({'status':0, 'version': applications[app_ind].version})

class deploy_application(tornado.web.RequestHandler):
  def get(self, app_id):
    global applications
    global location_tree
    global node_infos
    try:
      # Discovery results
      node_infos = location_tree.getAllNodeInfos()

      app_ind = getAppIndex(app_id)
      if app_ind == None:
        self.content_type = 'application/json'
        self.write({'status':1, 'mesg': 'Cannot find the application'})
      else:
        deployment = template.Loader(os.getcwd()).load('templates/deployment.html').generate(
                app=applications[app_ind],
                app_id=app_id, node_infos=node_infos,
                logs=applications[app_ind].logs(),
                changesets=applications[app_ind].changesets, 
                set_location=False, 
                default_location=LOCATION_ROOT)
        self.content_type = 'application/json'
        self.write({'status':0, 'page': deployment})
      
    except Exception as e:
      exc_type, exc_value, exc_traceback = sys.exc_info()
      print traceback.print_exception(exc_type, exc_value, exc_traceback,
                                      limit=2, file=sys.stdout)
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot initiate connection with the baseStation'})

  def post(self, app_id):
    global location_tree
    app_ind = getAppIndex(app_id)

    set_wukong_status("Start deploying")
    applications[app_ind].status = "    "
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      platforms = ['avr_mega2560']
      # signal deploy in other greenlet task
      wusignal.signal_deploy(platforms)
      set_active_application_index(app_ind)
         
      self.content_type = 'application/json'
      self.write({'status':0, 'version': applications[app_ind].version})

class map_application(tornado.web.RequestHandler):
  def post(self, app_id):
    global applications
    global location_tree
    global routingTable

    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      platforms = ['avr_mega2560']
      # TODO: need platforms from fbp

      # Map with location tree info (discovery), this will produce mapping_results
      applications[app_ind].map(location_tree, routingTable)

    #  print applications[app_ind].mapping_results

      ret = []
      for component in applications[app_ind].changesets.components:
        for ind, wuobj in enumerate(component.instances):
          if ind == 0:
            ret.append({'leader': True, 'instanceId': component.index,
                    'name': wuobj.wuclass.name, 'nodeId': wuobj.node_id,
                    'portNumber': wuobj.port_number})
          else:
            ret.append({'leader': False, 'instanceId': component.index, 'name':
                    wuobj.wuclass.name, 'nodeId': wuobj.node_id, 'portNumber':
                    wuobj.port_number})

      self.content_type = 'application/json'
      self.write({'status':0, 'mapping_results': ret, 'version': applications[app_ind].version})

class monitor_application(tornado.web.RequestHandler):
  def get(self, app_id):
    global applications
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

      monitor = template.Loader(os.getcwd()).load('templates/monitor.html').generate(app=applications[app_ind], logs=applications[app_ind].logs(), properties_json=properties_json)
      self.content_type = 'application/json'
      self.write({'status':0, 'page': monitor})

class properties_application(tornado.web.RequestHandler):
  def post(self, app_id):
    global applications
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
    global applications
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      #if int(applications[app_ind].version) <= int(self.get_argument('version')):
        #self.content_type = 'application/json'
        #self.write({'status':0, 'version': applications[app_ind].version, 'returnCode': applications[app_ind].returnCode, 'logs': applications[app_ind].retrieve()})
      #else:

      logging.info(get_wukong_status())
      wukong_status = get_wukong_status()
      set_wukong_status("")

      self.content_type = 'application/json'
      self.write({'status':0, 'version': applications[app_ind].version,
          'wukong_status': wukong_status, 
          'application_status': applications[app_ind].status, 
          'returnCode': applications[app_ind].returnCode, 
          'logs': applications[app_ind].retrieve()})

class save_fbp(tornado.web.RequestHandler):
  def post(self, app_id):
    global applications
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      xml = self.get_argument('xml')
      applications[app_ind].updateXML(xml)
      #applications[app_ind] = load_app_from_dir(applications[app_ind].dir)
      #applications[app_ind].xml = xml
      # TODO: need platforms from fbp
      #platforms = self.get_argument('platforms')
      platforms = ['avr_mega2560']

      self.content_type = 'application/json'
      self.write({'status':0, 'version': applications[app_ind].version})

class load_fbp(tornado.web.RequestHandler):
  def get(self, app_id):
    self.render('templates/fbp.html')

  def post(self, app_id):
    global applications
    app_ind = getAppIndex(app_id)
    if app_ind == None:
      self.content_type = 'application/json'
      self.write({'status':1, 'mesg': 'Cannot find the application'})
    else:
      self.content_type = 'application/json'
      self.write({'status':0, 'xml': applications[app_ind].xml})

class poll_testrtt(tornado.web.RequestHandler):
  def post(self):
    global applications
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
    global applications
    comm = getComm()
    if comm.onStopMode():
      self.content_type = 'application/json'
      self.write({'status':0})
    else:
      self.content_type = 'application/json'
      self.write({'status':1})

class exclude_testrtt(tornado.web.RequestHandler):
  def post(self):
    global applications
    comm = getComm()
    if comm.onDeleteMode():
      self.content_type = 'application/json'
      self.write({'status':0, 'log': 'Going into exclude mode'})
    else:
      self.content_type = 'application/json'
      self.write({'status':1, 'log': 'There is an error going into exclude mode'})

class include_testrtt(tornado.web.RequestHandler):
  def post(self):
    global applications
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
    node_infos = comm.getAllNodeInfos()

    testrtt = template.Loader(os.getcwd()).load('templates/testrtt.html').generate(log=['Please press the buttons to add/remove nodes.'], node_infos=node_infos, set_location=True, default_location = LOCATION_ROOT)
    self.content_type = 'application/json'
    self.write({'status':0, 'testrtt':testrtt})

class refresh_nodes(tornado.web.RequestHandler):
  def post(self):
    global location_tree
    global routingTable
    global node_infos

    comm = getComm()
    node_infos = comm.getActiveNodeInfos(force=True)
    logging.info("building tree from discovery")
    location_tree.buildTree(node_infos)
    #furniture data loaded from fake data for purpose of 
    location_tree.printTree()
    logging.info("getting routing information")
    routingTable = getComm().getRoutingInformation()
    logging.info(routingTable)
    # default is false
    set_location = self.get_argument('set_location', False)

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
    global location_tree
    global node_infos
    location = self.get_argument('location')
    if location:
      comm = getComm()
      if comm.setLocation(int(nodeId), location):
        routingTable = comm.getRoutingInformation()
        # update our knowledge too
        for info in comm.getActiveNodeInfos():
          if info.id == int(nodeId):
            info.location = location
            info.save()
            senNd = SensorNode(info)
            print (info.location)
            location_tree.addSensor(senNd)
        location_tree.printTree()
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
		list = self.write(str(glob.glob('/dev/tty*') + glob.glob('/dev/cu*')))
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
		type = self.get_argument('type')
		if type == 'C':
			name = self.get_argument('src')+'.c'
		elif type == 'JAVA':
			name = self.get_argument('src')+'.java'
		f = open(self.findPath(name))
		cont = f.read()
		f.close()
	except:
		self.write(traceback.format_exc())
		return
	self.write(cont)
  def post(self):
	try:
		print 'xxx'
		name = self.get_argument('name')
		type = self.get_argument('type')
		if type == 'C':
			name = name + '.c'
		elif type == 'Java':
			name = name + '.java'
		print 'name=',name
		f = open(self.findPath(name),'w')
		f.write(self.get_argument('content'))
		f.close()
		self.write('OK')
	except:
		self.write('Error')
		print traceback.format_exc()
	
  def findPath(self,p):
	name = '../../src/lib/wkpf/c/arduino/native_wuclasses/'+p
	if os.path.isfile(name):
		return name
	name = '../../src/lib/wkpf/c/common/native_wuclasses/'+p
	if os.path.isfile(name):
		return name
	name = p
	print 'yyyyy'
	return name
	
	

class tree(tornado.web.RequestHandler):	
  def post(self):
    global location_tree
    global node_infos
    
    load_xml = ""
    flag = os.path.exists("../ComponentDefinitions/landmark.xml")
#    if(flag):
    if(False):
      f = open("../ComponentDefinitions/landmark.xml","r")
      for row in f:
        load_xml += row	
    else:
      pass
    print node_infos      
    addloc = template.Loader(os.getcwd()).load('templates/display_locationTree.html').generate(node_infos=node_infos)

    location_tree.printTree()
    disploc = location_tree.getJson()

    self.content_type = 'application/json'
    self.write({'loc':json.dumps(disploc),'node':addloc,'xml':load_xml})

class save_tree(tornado.web.RequestHandler):
    def put(self):
        global location_tree
        
        self.write({'tree':location_tree})

    def post(self):
        landmark_info = self.get_argument('xml')
        f = open("../ComponentDefinitions/landmark.xml","w")
        f.write(landmark_info)
        f.close()
        
class add_landmark(tornado.web.RequestHandler):
  def put(self):
    global location_tree
    global landId

    name = self.get_argument("name")
    location = self.get_argument("location")
    operation = self.get_argument("ope")
    
    if(operation=="1"):
      landId += 1
      landmark = LandmarkNode(landId, name, location, 0) 
      location_tree.addLandmark(landmark)
      location_tree.printTree()
#    elif(operation=="0")
#      location_tree.delLandmark()
    
    self.content_type = 'application/json'
    self.write({'status':0})

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
  (r"/nodes/refresh", refresh_nodes),
  (r"/applications", list_applications),
  (r"/applications/new", new_application),
  (r"/applications/([a-fA-F\d]{32})", application),
  (r"/applications/([a-fA-F\d]{32})/reset", reset_application),
  (r"/applications/([a-fA-F\d]{32})/properties", properties_application),
  (r"/applications/([a-fA-F\d]{32})/poll", poll),
  (r"/applications/([a-fA-F\d]{32})/deploy", deploy_application),
  (r"/applications/([a-fA-F\d]{32})/deploy/map", map_application),
  (r"/applications/([a-fA-F\d]{32})/monitor", monitor_application),
  (r"/applications/([a-fA-F\d]{32})/fbp/save", save_fbp),
  (r"/applications/([a-fA-F\d]{32})/fbp/load", load_fbp),
  (r"/loc_tree", tree),
  (r"/loc_tree/save", save_tree),
  (r"/loc_tree/land_mark", add_landmark),
  (r"/componentxml",WuLibrary),
  (r"/wuclasssource",WuClassSource),
  (r"/serialport",SerialPort),
  (r"/enablexml",EnabledWuClass)
], IP, **settings)

if __name__ == "__main__":
  logging.info("WuKong starting up...")
  setup_signal_handler_greenlet()
  if os.path.exists('standardlibrary.db'):
    os.remove('standardlibrary.db') 
  Parser.parseLibrary(COMPONENTXML_PATH)
  update_applications()
  import_wuXML()
  make_FBP()
  wukong.listen(MASTER_PORT)
  ioloop.start()
else: # so it could be called from test or other external libraries
  logging.info("WuKong starting up in tests...")
  setup_signal_handler_greenlet()
  update_applications()
  import_wuXML()
  make_FBP()
  wukong.listen(MASTER_PORT)
