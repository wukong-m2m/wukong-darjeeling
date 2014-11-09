sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

import traceback
import shutil, errno
import platform
import os, sys, zipfile, re, time
import tornado.web
import tornado.template as template
import simplejson as json

from sysmanager import *

sysmanager = SystemManager.init()

class RefreshNodes(tornado.web.RequestHandler):
    def post(self, force):
        if int(force,0) == 0:
            sysmanager.refreshNodeInfo(False)
        else:
            sysmanager.refreshNodeInfo(True)

        set_location = self.get_argument('set_location', False)
        if set_location == u'True':
            set_location = True
        else:
            set_location = False

        nodes = template.Loader(os.getcwd()).load('templates/monitor-nodes.html').generate(
            node_infos=sysmanager.getNodeInfos(), set_location=set_location, default_location=LOCATION_ROOT)

        self.content_type = 'application/json'
        self.write({'status':0, 'nodes': nodes})

class Nodes(tornado.web.RequestHandler):
    def get(self):
        pass

    def post(self, nodeId):
        info = sysmanager.getNodeInfo(nodeId)
        self.content_type = 'application/json'
        self.write({'status':0, 'node_info': info})

    def put(self, nodeId):

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
            info = sysmanager.getNodeInfo(nodeId)
            #print "device type=",info.type
            if info.type == 'native':
                info.location = location
                WuNode.saveNodes()
                senNd = SensorNode(info)
                wkpf.globals.location_tree.addSensor(senNd)
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
        appId = self.get_argument('appid')
        app = appmanager.getApplication(appId)
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
        appId = self.get_argument('appid')
        app = appmanager.getApplication(appId)
        try:
            component_path = app.dir+'/WKDeployCustomComponents.xml'
            f = open(component_path, 'w')
            xml = f.write(xml)
            f.close()
            sysmanager.make_main(component_path)
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
                app = appmanager.getApplication(appid)

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
                app = appmanager.getApplication(appid)

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