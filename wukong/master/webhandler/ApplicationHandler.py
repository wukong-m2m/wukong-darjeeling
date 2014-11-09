import os, sys, zipfile, re, time
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
import tornado.web
import tornado.template as template
import simplejson as json
import traceback
import hashlib
import logging
from configuration import *
from manager.ApplicationManager import ApplicationManager
from manager.SystemManager import SystemManager
from manager.ModelManager import ModelManager

appmanager = ApplicationManager.init()
sysmanager = SystemManager.init()
modelmanager = ModelManager.init()

# Returns a form to upload new application
class CreateApplication(tornado.web.RequestHandler):
    def post(self):
        try:
            try:
                app_name = self.get_argument('app_name')
            except:
                app_name = 'application' + sysmanager.getApplicationSize()
                app_id = hashlib.md5(app_name).hexdigest()
                if appmanager.hasApplication(app_id):
                    self.content_type = 'application/json'
                    self.write({'status':1, 'mesg':'Cannot create application with the same name'})
                    return

                app = appmanager.createApplication(app_id);
                self.content_type = 'application/json'
                self.write({'status':0, 'app': app.config()})
        except Exception as e:
            exc_type, exc_value, exc_traceback = sys.exc_info()
            print traceback.print_exception(exc_type, exc_value, exc_traceback,
                                  limit=2, file=sys.stdout)
            self.content_type = 'application/json'
            self.write({'status':1, 'mesg':'Cannot create application'})

# Rename an application
class RenameApplication(tornado.web.RequestHandler):
    def put(self, app_id):
        if not appmanager.hasApplication(app_id):
            self.content_type = 'application/json'
            self.write({'status':1, 'mesg': 'Cannot find the application'})
        else:
            try:
                appmanager.renameApplication(app_id, self.get_argument('value', ''))
                self.content_type = 'application/json'
                self.write({'status':0})
            except Exception as e:
                exc_type, exc_value, exc_traceback = sys.exc_info()
                print traceback.print_exception(exc_type, exc_value, exc_traceback,
                                      limit=2, file=sys.stdout)
                self.set_status(400)
                self.content_type = 'application/json'
                self.write({'status':1, 'mesg': 'Cannot save application'})

class Application(tornado.web.RequestHandler):
    # topbar info
    def get(self, app_id):
        if not appmanager.hasApplication(app_id):
            self.content_type = 'application/json'
            self.write({'status':1, 'mesg': 'Cannot find the application'})
        else:
            title = ""
            if self.get_argument('title'):
                title = self.get_argument('title')
            app = appmanager.getApplication(app_id)
            topbar = template.Loader(os.getcwd()).load('templates/topbar.html').generate(application=app, title=title, default_location=LOCATION_ROOT)
            self.content_type = 'application/json'
        self.write({'status':0, 'app': app.config(), 'topbar': topbar})

    # Display a specific application
    def post(self, app_id):
        if not appmanager.hasApplication(app_id):
            self.content_type = 'application/json'
            self.write({'status':1, 'mesg': 'Cannot find the application'})
        else:
            # active application
            appmanager.setActiveApplicationIndex(app_id)
            app = appmanager.getApplication(app_id)
            topbar = template.Loader(os.getcwd()).load('templates/topbar.html').generate(application=app, title="Flow Based Programming")
            self.content_type = 'application/json'
            self.write({'status':0, 'app': app.config(), 'topbar': topbar})

    # Update a specific application
    def put(self, app_id):
        if not appmanager.hasApplication(app_id):
            self.content_type = 'application/json'
            self.write({'status':1, 'mesg': 'Cannot find the application'})
        else:
            try:
                appmanager.updateApplication(app_id, self.get_argument('name', ''), self.get_argument('desc', ''))
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
        if not appmanager.hasApplication(app_id):
            self.content_type = 'application/json'
            self.write({'status':1, 'mesg': 'Cannot find the application'})
        else:
            if sysmanager.deleteApplication(app_ind):
                self.content_type = 'application/json'
                self.write({'status':0})
            else:
                self.content_type = 'application/json'
                self.write({'status':1, 'mesg': 'Cannot delete application'})

class ResetApplication(tornado.web.RequestHandler):
    def post(self, app_id):
        if not appmanager.hasApplication(app_id):
            self.content_type = 'application/json'
            self.write({'status':1, 'mesg': 'Cannot find the application'})
        else:
            sysmanager.setWukongStatus("close")
            app = appmanager.getApplication(app_id)
            app.status = "close"
            self.content_type = 'application/json'
            self.write({'status':0, 'version': app.version})

class DeployApplication(tornado.web.RequestHandler):
    def get(self, app_id):
        if not appmanager.hasApplication(app_id):
            self.content_type = 'application/json'
            self.write({'status':1, 'mesg': 'Cannot find the application'})
        else:
            app = appmanager.getApplication(app_id)
             # deployment.js will call refresh_node eventually, rebuild location tree there
            deployment = template.Loader(os.getcwd()).load('templates/deployment.html').generate(
                app=app,
                app_id=app_id, node_infos=sysmanager.getNodeInfos(),
                logs=app.logs(),
                changesets=app.changesets,
                set_location=False,
                default_location=LOCATION_ROOT)
            self.content_type = 'application/json'
            self.write({'status':0, 'page': deployment})

    def post(self, app_id):
        if not appmanager.hasApplication(app_id):
            self.content_type = 'application/json'
            self.write({'status':1, 'mesg': 'Cannot find the application'})
        else:
            sysmanager.setWukongStatus("Deploying")
            platforms = ['avr_mega2560']
            sysmanager.signalDeploy(app_ind, platforms)

            self.content_type = 'application/json'
            self.write({
                'status':0,
                'version': appmanager.getApplication(app_id).version})

class MapApplication(tornado.web.RequestHandler):
    def post(self, app_id):
        if not appmanager.hasApplication(app_id):
            self.content_type = 'application/json'
            self.write({'status':1, 'mesg': 'Cannot find the application'})
        else:
            platforms = ['avr_mega2560']
            # TODO: need platforms from fbp
            sysmanager.refreshNodeInfo()

            # Map with location tree info (discovery), this will produce mapping_results
            mapping_result = appmanager.getApplication(app_id).map(modelmanager.getLocationTree(), [])

            ret = []
            app = appmanager.getApplication(app_id)
            for component in app.changesets.components:
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
                'version': app.version,
                'mapping_status': app.mapping_status})

class MonitorApplication(tornado.web.RequestHandler):
    def get(self, app_id):
        if not appmanager.hasApplication(app_id):
            self.content_type = 'application/json'
            self.write({'status':1, 'mesg': 'Cannot find the application'})
        else:
            properties_json = WuProperty.all() # for now
            app = appmanager.getApplication(app_id)
            monitor = template.Loader(
                os.getcwd()).load('templates/monitor.html').generate(app=app, logs=app.logs(), properties_json=properties_json)
            self.content_type = 'application/json'
            self.write({'status':0, 'page': monitor})

class ListApplicationProperties(tornado.web.RequestHandler):
    def post(self, app_id):
        if not appmanager.hasApplication(app_id):
            self.content_type = 'application/json'
            self.write({'status':1, 'mesg': 'Cannot find the application'})
        else:
            properties_json = WuProperty.all() # for now
            self.content_type = 'application/json'
            self.write({'status':0, 'properties': properties_json})

# Never let go
class PollApplication(tornado.web.RequestHandler):
    def post(self, app_id):
        if not appmanager.hasApplication(app_id):
            self.content_type = 'application/json'
            self.write({'status':1, 'mesg': 'Cannot find the application'})
        else:
            app = sysmanager.getApplication(app_id)
            self.content_type = 'application/json'
            self.write({
                'status':0,
                'ops': app.deploy_ops,
                'version': app.version,
                'deploy_status': app.deploy_status,
                'mapping_status': app.mapping_status,
                'wukong_status': sysmanager.getWukongStatus(),
                'application_status': app.status,
                'returnCode': app.returnCode})

        # TODO: log should not be requested in polling, should be in a separate page
        # dedicated for it
        # because logs could go up to 10k+ entries
        #'logs': wkpf.globals.applications[app_ind].logs()