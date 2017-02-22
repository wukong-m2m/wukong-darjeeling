#!/usr/bin/python
# vim: ts=2 sw=2 expandtab

# author: Penn Su
# Modified:
#   Hsin Yuan Yeh (iapyeh@gmail.com)
#   Sep 18, April 18, May 13, 2015
#
import sys
reload(sys)  # Reload does the trick!
sys.setdefaultencoding('UTF8')
from gevent import monkey; monkey.patch_all()
import json
import gevent
import platform
import os, zipfile, re, time
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
import urllib
import urlparse
import datetime
import glob
import copy
import fcntl, termios, struct
from modules.node_red import node_red
from modules.device_manager import device_manager
from modules.fbp_editor import fbp_editor
from modules.map_and_deploy import map_and_deploy
from modules.monitoring import monitoring
from modules.location_tree import location_tree
from modules.progression import progression

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
from wkpf.model.models import *

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

def make_FBP():
  test_1 = fbp_main()
  test_1.make()

wkpf.globals.location_tree = LocationTree(LOCATION_ROOT)
from wkpf.model.locationParser import LocationParser
wkpf.globals.location_parser = LocationParser(wkpf.globals.location_tree)

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
    self.render('templates/application2.html', connected=wkpf.globals.connected)


settings = dict(
  static_path=os.path.join(os.path.dirname(__file__), "static"),
  debug=True
)

ioloop = tornado.ioloop.IOLoop.instance()
wukong = tornado.web.Application([
  (r"/", main),
  (r"/ide", idemain),
  (r"/main", main),
  (r"/testrtt/exclude", device_manager.exclude_testrtt),
  (r"/testrtt/include", device_manager.include_testrtt),
  (r"/testrtt/stop", device_manager.stop_testrtt),
  (r"/testrtt/poll", device_manager.poll_testrtt),
  (r"/testrtt", device_manager.testrtt),
  (r"/nodes/([0-9]*)", device_manager.nodes),
  (r"/nodes/refresh/([0-9])", device_manager.refresh_nodes),
  (r"/applications", fbp_editor.list_applications),
  (r"/applications/new", fbp_editor.new_application),
  (r"/applications/([a-fA-F\d]{32})", fbp_editor.application),
  (r"/applications/([a-fA-F\d]{32})/rename", fbp_editor.rename_application),
  (r"/applications/([a-fA-F\d]{32})/disable", fbp_editor.disable_application),
  (r"/applications/([a-fA-F\d]{32})/reset", fbp_editor.reset_application),
  (r"/applications/([a-fA-F\d]{32})/poll", fbp_editor.poll),
  (r"/applications/([a-fA-F\d]{32})/deploy", map_and_deploy.deploy_application),
  (r"/applications/([a-fA-F\d]{32})/deploy/map", map_and_deploy.map_application),
  (r"/applications/([a-fA-F\d]{32})/fbp/save", fbp_editor.save_fbp),
  (r"/applications/([a-fA-F\d]{32})/fbp/load", fbp_editor.load_fbp),
  (r"/applications/([a-fA-F\d]{32})/fbp/download", fbp_editor.download_fbp),
  (r"/applications/([a-fA-F\d]{32})/fbp/submit2appstore", fbp_editor.Submit2AppStore),
  (r"/applications/([a-fA-F\d]{32})/fbp/read_signal", node_red.ReadMessageFromNodeRed),
  (r"/appstore/remove", fbp_editor.RemoveAppFromStore),
  (r"/loc_tree/nodes/([0-9]*)", location_tree.loc_tree),
  (r"/loc_tree/edit", location_tree.edit_loc_tree),
  (r"/loc_tree/parse", location_tree.loc_tree_parse_policy),
  (r"/loc_tree/nodes/([0-9]*)/(\w+)", location_tree.sensor_info),
  (r"/loc_tree", location_tree.loc_tree),
  (r"/loc_tree/modifier/([0-9]*)", location_tree.tree_modifier),
  (r"/loc_tree/save", location_tree.save_landmark),
  (r"/loc_tree/load", location_tree.load_landmark),
  (r"/loc_tree/land_mark", location_tree.add_landmark),
  (r"/monitoring", monitoring.Monitoring),
  (r"/monitoring_chart/([0-9]*)/([0-9]*)", monitoring.Monitoring_Chart),
  (r"/monitoring_planar", monitoring.Monitoring_Planar),
  (r"/configuration", progression.Progression),
  (r"/nodered/inputfrom", node_red.NodeRedInputFrom),
  (r"/nodered/outputto", node_red.NodeRedOutputTo),
  (r"/nodered/read", node_red.ReadMessageToNodeRed)
], IP, **settings)

logging.info("Starting up...")
setup_signal_handler_greenlet()
WuClassLibraryParser.read(COMPONENTXML_PATH)
#WuNode.loadNodes()
update_applications()
make_main()
make_FBP()
getComm()
wukong.listen(MASTER_PORT)

if __name__ == "__main__":
  ioloop.start()
