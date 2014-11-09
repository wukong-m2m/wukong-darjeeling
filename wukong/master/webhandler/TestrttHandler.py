import os, sys, zipfile, re, time
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
import tornado.web
import tornado.template as template
from configuration import *
from manager.SystemManager import SystemManager
import glob

sysmanager = SystemManager.init()

class PollTestrtt(tornado.web.RequestHandler):
    def post(self):
        status = sysmanager.getCurrentCommStatus()
        if status != None:
            self.content_type = 'application/json'
            self.write({'status':0, 'logs': status.split('\n')})
        else:
            self.content_type = 'application/json'
            self.write({'status':0, 'logs': []})

class StopTestrtt(tornado.web.RequestHandler):
    def post(self):
        systemmanager.refreshNodeInfo()
        if sysmanager.onStopMode():
            self.content_type = 'application/json'
            self.write({'status':0})
        else:
            self.content_type = 'application/json'
            self.write({'status':1})

class ExcludeTestrtt(tornado.web.RequestHandler):
    def post(self):
        if sysmanager.onDeleteMode():
            self.content_type = 'application/json'
            self.write({'status':0, 'log': 'Going into exclude mode'})
        else:
            self.content_type = 'application/json'
            self.write({'status':1, 'log': 'There is an error going into exclude mode'})

class IncludeTestrtt(tornado.web.RequestHandler):
    def post(self):
        if sysmanager.onAddMode():
            self.content_type = 'application/json'
            self.write({'status':0, 'log': 'Going into include mode'})
        else:
            self.content_type = 'application/json'
            self.write({'status':1, 'log': 'There is an error going into include mode'})

class Testrtt(tornado.web.RequestHandler):
    def get(self):
        sysmanager.refreshNodeInfo()
        testrtt = template.Loader(os.getcwd()).load('templates/testrtt.html').generate(log=['Please press the buttons to add/remove nodes.'], node_infos=sysmanager.getNodeInfos(), set_location=True, default_location = LOCATION_ROOT)
        self.content_type = 'application/json'
        self.write({'status':0, 'testrtt':testrtt})