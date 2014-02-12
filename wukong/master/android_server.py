#!/usr/bin/python
#Author: Anastasia Shuba

import BaseHTTPServer, hashlib, logging, shutil, traceback
import sys, os, copy, errno
from multiprocessing import Queue

import tornado.template as template

import wkpf.globals
from wkpf.wuclasslibraryparser import *
from wkpf.wkpfcomm import *
from wkpf.util import *
from configuration import *

from configuration import *
from wkpf.wuapplication import WuApplication
import wkpf.wusignal as wusignal
from wkpf.locationTree import LocationTree

#Constants:
HOST_NAME = '192.168.1.83' #Home
PORT_NUMBER = 6789

#Send command will always be a POST request
DEPLOY_PICK_COMMAND = "1"
DEPLOY_APP_COMMAND = "2"
STOP_COMMAND = "3"
VIEW_COMMAND = "4"

def start_android_server():
	server_class = BaseHTTPServer.HTTPServer
	httpd = server_class((HOST_NAME, PORT_NUMBER), AndroidServer)
	print "Android Server Started - %s:%s" % (HOST_NAME, PORT_NUMBER)
	try:
		httpd.serve_forever()
	except KeyboardInterrupt:
		pass
	httpd.server_close()
	print "Android Server Stopped - %s:%s" % (HOST_NAME, PORT_NUMBER)

# using cloned nodes
def rebuildTree(nodes):
	nodes_clone = copy.deepcopy(nodes)
	wkpf.globals.location_tree = LocationTree(LOCATION_ROOT)
	wkpf.globals.location_tree.buildTree(nodes_clone)
	flag = os.path.exists("../LocalData/landmarks.txt")
	if(flag):
		wkpf.globals.location_tree.loadTree()
	wkpf.globals.location_tree.printTree()

def getAppIndex(app_id):
	# make sure it is not unicode
	app_id = app_id.encode('ascii','ignore')
	for index, app in enumerate(wkpf.globals.applications):
		if app.id == app_id:
			return index
	return None

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

# Destroy a specific application
def delete(app_id, msg_writer):
	app_ind = getAppIndex(app_id)
	if app_ind == None:
		msg_writer.write({'status':1, 'mesg': 'Cannot find the application'})
	else:
		if delete_application(app_ind):
			msg_writer.write({'status':0})
		else:
			msg_writer.write({'status':1, 'mesg': 'Cannot delete application'})

def save_new_app(app_name, msg_writer, xml=None):
	app_id = hashlib.md5(app_name).hexdigest()
	if getAppIndex(app_id) is not None:
		msg_writer.write({'status':1, 'mesg':'Cannot create application with the same name'})
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
	if xml:
		app_ind = getAppIndex(app_id) #Get the new index
		try:
			app.updateXML(xml) #save xml
		except Exception as e:
			msg_writer.write({'status':1, 'mesg': 'Error parsing xml. Please send only WuKong FBPs.'})
			delete(app_id, msg_writer) #Don't save bad app
			return
	msg_writer.write({'status':0, 'app': app.config()})
	return app_id

def deploy_app(app_id, msg_writer):
	#TODO: hardcoded here, differs from original
	node_infos = getComm().getActiveNodeInfos(False)

	rebuildTree(node_infos)
	print ("node_infos in refresh nodes:",node_infos)

	#MAP:
	app_ind = getAppIndex(app_id)
	if app_ind == None:
		msg_writer.write({'status':1, 'mesg': 'Cannot find the application'})
		return
	platforms = ['avr_mega2560']
	rebuildTree(node_infos)

	mapping_result = wkpf.globals.applications[app_ind].map(wkpf.globals.location_tree, [])

	ret = []
	for component in wkpf.globals.applications[app_ind].changesets.components:
		obj_hash = {
		'instanceId': component.index,
		'location': component.location,
		'group_size': component.group_size,
		'name': component.type,
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

	#DEPLOY
	wkpf.globals.set_wukong_status("Deploying")
	platforms = ['avr_mega2560']
	# signal deploy in other greenlet task
	wusignal.signal_deploy(platforms)
	wkpf.globals.set_active_application_index(app_ind)

	#Note: this part differs for Android/regular deploy
	queue = Queue()
	wusignal.set_android_queue(queue)
	response = queue.get()
	#TODO: not sure if it's because app fails, but
	#trying to deploy another app just gets the thing stuck
	#Same with non-android deploy it seems

	if response == True: #all is well
		msg_writer.write({'status':0})
	else:
		msg_writer.write({'status':1, 'mesg':'Could not deploy application.'})

class AndroidServer(BaseHTTPServer.BaseHTTPRequestHandler):

	def do_HEAD(self):
		self.send_response(200)
		self.send_header("Content-type", "text/html")
		self.end_headers()
	def do_GET(self):
		"""Respond to a GET request."""
		self.send_response(200)
		self.send_header("Content-type", "text/html")
		self.end_headers()

		cmd = self.path.split("/")[1]
		if(cmd == DEPLOY_PICK_COMMAND):
			app_names = []
			for app in list(wkpf.globals.applications):
				app_names.append(str(app.app_name))
			self.wfile.write({'status':0, 'apps':app_names})

		elif(cmd == DEPLOY_APP_COMMAND):
			print "*** About to deploy..."
			app_name = self.headers.getheader("file_name")
			app_id = hashlib.md5(app_name).hexdigest()
			print "*** About to deploy... %s ***" % app_id
			deploy_app(app_id, self.wfile)
		elif(cmd == STOP_COMMAND):
			print "TODO Stop"
			for app in list(wkpf.globals.applications):
				print app.app_name
		elif(cmd == VIEW_COMMAND):
			print "TODO View"
		#Send resposne string to client:
		#self.wfile.write("true")

	#This method will be used upon a SEND command only
	def do_POST(self):
		global latest_app_id
		self.send_response(200)
		self.send_header("Content-type", "text/html")
		self.end_headers()
		app_name = self.headers.getheader("file_name")
		xml = self.rfile.read(int(self.headers.getheader("content-length")))
		latest_app_id = save_new_app(app_name, self.wfile, xml)
		print "*** Done saving app %s ***" % latest_app_id
