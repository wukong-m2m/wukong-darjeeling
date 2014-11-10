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
import traceback
import shutil, errno
import datetime
import glob
import copy
import fcntl, termios, struct
from configuration import *
from wkpf.wuclasslibraryparser import *

WuClassLibraryParser.read(COMPONENTXML_PATH)

from webhandler.ApplicationHandler import *
from webhandler.ConfigurationHandler import *
from webhandler.FbpHandler import *
from webhandler.LocationHandler import *
from webhandler.MonitorHandler import *
from webhandler.StaticPageHandler import *
from webhandler.TestrttHandler import *
from manager.SystemManager import *

import tornado.options
tornado.options.define("appdir", type=str, help="Directory that contains the applications")
tornado.options.parse_command_line()

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

IP = sys.argv[1] if len(sys.argv) >= 2 else '127.0.0.1'

ioloop = tornado.ioloop.IOLoop.instance()
wukong = tornado.web.Application([
  (r"/", Home),
  (r"/ide", Editor),
  (r"/main", Home),
  (r"/testrtt/exclude", ExcludeTestrtt),
  (r"/testrtt/include", IncludeTestrtt),
  (r"/testrtt/stop", StopTestrtt),
  (r"/testrtt/poll", PollTestrtt),
  (r"/testrtt", Testrtt),
  (r"/nodes/([0-9]*)", Nodes),
  (r"/nodes/refresh/([0-9])", RefreshNodes),
  (r"/applications", Applications),
  (r"/applications/new", CreateApplication),
  (r"/applications/([a-fA-F\d]{32})", Application),
  (r"/applications/([a-fA-F\d]{32})/rename", RenameApplication),
  (r"/applications/([a-fA-F\d]{32})/reset", ResetApplication),
  (r"/applications/([a-fA-F\d]{32})/properties", ListApplicationProperties),
  (r"/applications/([a-fA-F\d]{32})/poll", PollApplication),
  (r"/applications/([a-fA-F\d]{32})/deploy", DeployApplication),
  (r"/applications/([a-fA-F\d]{32})/deploy/map", MapApplication),
  (r"/applications/([a-fA-F\d]{32})/monitor", MonitorApplication),
  (r"/applications/([a-fA-F\d]{32})/fbp/save", SaveFBP),
  (r"/applications/([a-fA-F\d]{32})/fbp/load", LoadFBP),
  (r"/loc_tree/nodes/([0-9]*)", DisplayLocationTree),
  (r"/loc_tree/edit", EditLocationTree),
  (r"/loc_tree/nodes/([0-9]*)/(\w+)", SensorInfo),
  (r"/loc_tree", DisplayLocationTree),
  (r"/loc_tree/modifier/([0-9]*)", TreeModifier),
  (r"/loc_tree/save", SaveLandMark),
  (r"/loc_tree/load", LoadLandMark),
  (r"/loc_tree/land_mark", AddLandMark),
  (r"/componentxml",WuLibrary),
  (r"/componentxmluser",WuLibraryUser),
  (r"/wuclasssource",WuClassSource),
  (r"/serialport",SerialPort),
  (r"/enablexml",EnabledWuClass),
  (r"/build",Build),
  (r"/upload",Upload)
  ,(r"/monitoring",Monitoring)
  ,(r"/getvalue",GetValue)
], IP, **settings)


if __name__ == "__main__":
  SystemManager.init()
  wukong.listen(MASTER_PORT)
  ioloop.start()

