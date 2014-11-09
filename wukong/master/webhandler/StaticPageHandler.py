sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

import tornado.web
import tornado.template as template
from wkpf.wkpfcomm import *
import wkpf.globals

sysmanager = SystemManager.init()

# Serve WuClass Editor
class Editor(tornado.web.RequestHandler):
    def get(self):
        self.content_type='text/html'
        self.render('templates'/ide.html)

# Serve Home Page
class Home(tornado.web.RequestHandler):
    def get(self):
        getComm()
        self.render('../templates/application.html', connected=wkpf.globals.connected)

# List all application Page
class Applications(tornado.web.RequestHandler):
  def get(self):
    self.render('../templates/index.html', applications=sysmanager.getAllApplications())

  def post(self):
    sysmanager.updateApplications()
    apps = sorted([application.config() for application in sysmanager.getApplications()], key=lambda k: k['app_name'])
    self.content_type = 'application/json'
    self.write(json.dumps(apps))