import traceback
import shutil, errno
import platform
import os, sys, zipfile, re, time
import tornado.web
import tornado.template as template
import simplejson as json

class Test:
  def __init__(self,name,n_id,pt,loc):
    self.id = str(n_id)+'_'+str(pt)
    self.sensor = name
    self.loc=loc
    self.value = wkpf.globals.mongoDBClient.wukong.readings.find({ 'node_id':n_id , 'port':pt }).sort('timestamp',-1).limit(1)[0]['value']
          #wkpf.globals.mongoDBClient.wukong.readings.find().sort({"timestamp":-1}).sort("timestamp":-1)['value']

class Monitoring(tornado.web.RequestHandler):
  def get(self):
      comm = getComm()
      node_infos = comm.getAllNodeInfos(False)
      # print node_infos
      for node in node_infos:
        print node.id
        print node.location
        for port_number in node.wuobjects.keys():
          wuobject = node.wuobjects[port_number]
          print 'port:', port_number
          print wuobject.wuclassdef.name

      obj1 = Test('IR Sensor',2,3,'BL-7F entrance')#location tree
      obj2 = Test('Ultra Sound Sensor',3,4,'BL-7F entrance')
      self.render('templates/index2.html', applications=[obj1,obj2])

  def post(self):
    apps=wkpf.globals.mongoDBClient.wukong.readings.find().sort('timestamp',-1).limit(1)[2]
    #apps = sorted([application.config() for application in wkpf.globals.applications], key=lambda k: k['app_name'])
    self.content_type = 'application/json'
    self.write(json.dumps(apps))

class GetValue(tornado.web.RequestHandler):
  def get(self):
      #obj1 = Test('IR Sensor',9,3,'BL-7F entrance')
      #obj2 = Test('Ultra Sound Sensor',11,4,'BL-7F entrance')
      obj2 = Test('IR Sensor',int(self.get_argument("arg2")),int(self.get_argument("arg3")),'BL-7F entrance')
      #self.render('templates/value.html', applications=[obj1,obj2])
      self.render('templates/value.html', applications=[obj2.value])