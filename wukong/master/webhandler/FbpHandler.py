import traceback
import shutil, errno
import platform
import os, sys, zipfile, re, time
import tornado.web
import tornado.template as template
import simplejson as json
import manager.ApplicationManager
import manager.SystemManager

appmanager = ApplicationManager.init()
sysmanager = SystemManager.init()

class SaveFBP(tornado.web.RequestHandler):
    def post(self, app_id):
        if not appmanager.hasApplication(app_id):
            self.content_type = 'application/json'
            self.write({'status':1, 'mesg': 'Cannot find the application'})
        else:
            xml = self.get_argument('xml')
            app = appmanager.getApplication(app_id)
            app.updateXML(xml)
            platforms = ['avr_mega2560']
            self.content_type = 'application/json'
            self.write({'status':0, 'version': app.version})

class LoadFBP(tornado.web.RequestHandler):
    def get(self, app_id):
        self.render('templates/fbp.html')

    def post(self, app_id):
        if not appmanager.hasApplication(app_id):
            self.content_type = 'application/json'
            self.write({'status':1, 'mesg': 'Cannot find the application'})
        else:
            self.content_type = 'application/json'
            self.write({'status':0, 'xml': appmanager.getApplication(app_id).xml})