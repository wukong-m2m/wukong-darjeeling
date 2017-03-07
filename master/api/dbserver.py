#!/usr/bin/python
# vim: ts=2 sw=2 expandtab

# author: Penn Su
from bson.objectid import ObjectId
import dateutil.parser

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
from threading import Thread
import traceback
import StringIO
import shutil, errno
import datetime
from datetime import date, timedelta
import glob
import copy
import fcntl, termios, struct
from types import *

import tornado.options
tornado.options.define("appdir", type=str, help="Directory that contains the applications")
tornado.options.parse_command_line()
from configuration_db import *

import pymongo

import json

class JSONEncoder(json.JSONEncoder):
    def default(self, o):
      return str(o)
      '''
        if isinstance(o, ObjectId):
            return str(o)
        return json.JSONEncoder.default(self, o)
      '''



import tornado.options

print "[MONITORING]="+MONITORING+"[.]"
if(MONITORING == 'true'):
    try:
      from pymongo import MongoClient
    except:
      print "Please install python mongoDB driver pymongo by using"
      print "easy_install pymongo"
      sys.exit(-1)

    try:
        mongoDBClient = MongoClient(MONGODB_URL)
        print "MongoDB instance " + MONGODB_URL + " HAS be connected."
    except:
      print "MongoDB instance " + MONGODB_URL + " can't be connected."
      print "Please install the mongDB, pymongo module."
      sys.exit(-1)

tornado.options.parse_command_line()
#tornado.options.enable_pretty_logging()

IP = sys.argv[1] if len(sys.argv) >= 2 else '127.0.0.1'

landId = 100
node_infos = []

class asd(tornado.web.RequestHandler):
    def get(self):
        self.render('templates/asd.html', applications=[])

class MainHandler(tornado.web.RequestHandler):
    def get(self):
        self.write("Hello, world")

class Test:
  def __init__(self,name,n_id,pt,loc):
    self.id = str(n_id)+'_'+str(pt)
    self.n_id=str(n_id)
    self.pt=str(pt)
    self.sensor = name
    self.loc=loc
    self.value = mongoDBClient.wukong.readings.find({ 'node_id':n_id , 'port':pt }).sort('_id',-1).limit(1)[0]['value']
    self.ts = mongoDBClient.wukong.readings.find({ 'node_id':n_id , 'port':pt }).sort('_id',-1).limit(1)[0]['timestamp']


class GetValue(tornado.web.RequestHandler):
  def get(self):
      obj2 = Test('IR Sensor',int(self.get_argument("arg2")),int(self.get_argument("arg3")),'BL-7F entrance')
      self.render('templates/value.html', applications=[obj2.value])
      #http://localhost:8888/getvalue?arg1=IR%20Sensor&arg2=3&arg3=2&arg4=BL-7F%20entrance

#np={'wuclass_id': 3, 'port': 2}
class Test_doc:
  def __init__(self,np):
    print np
    self.result = []
    for it in np:
      ii1=time.time()
      tmp_result=list(mongoDBClient.wukong.readings.find(it).sort('_id',-1))
      for iit in tmp_result:
        iit['_id']=str(iit['_id'])
        iit['timestamp']=str(iit['timestamp'])
      self.result.append(tmp_result )
      
      ii2=time.time()
      print "[ININ][TIME] it take %f s For each (node/port) query" %(ii2-ii1)
    
    #self.value = list(mongoDBClient.wukong.readings.find(np).sort('_id',-1))[0]['value']
    #print self.value
    #self.ts = mongoDBClient.wukong.readings.find(np).sort('_id',-1).limit(10)[0]['timestamp']

#app / com / (att) / loc / wuc 
# "app_id" : df40477e9a75456ed53400041634f94b
# "name" : "Light_Sensor"
# ()
# "location" : "/WuKong/Door#"
# "classId" : 1001
#http://localhost:8888/get?app=df40477e9a75456ed53400041634f94b&com=Light_Sensor&loc=/WuKong/Door%23&wuc=1001

#node / port
#time_from / time_end

class Print_doc:
  def __init__(self,np,target):
    print np
    #self.result = []

    exec("self.result=list(mongoDBClient.wukong."+target+".find(np).sort('_id',-1))")
    #exec("self.result=list(mongoDBClient.wukong."+target+".find_one(np))")#test
    
    '''
    for iit in tmp_result:
      iit['_id']=str(iit['_id'])
      iit['timestamp']=str(iit['timestamp'])
    self.result.append(tmp_result )
    '''  


class Get(tornado.web.RequestHandler):
  def get(self):
    
    query = {}
    np = []
    timerange = {}
    t1 = time.time()
    if self.get_argument("app",default=None) is not None:
      query['app_id']=str(self.get_argument("app"))
    if self.get_argument("com",default=None) is not None:
      query['name']=str(self.get_argument("com"))
    if self.get_argument("loc",default=None) is not None:
      query['location']=str(self.get_argument("loc"))
    if self.get_argument("wuc",default=None) is not None:
      query['classId']=int(self.get_argument("wuc"))
    if self.get_argument("from",default=None) is not None:
      query['from']=int(self.get_argument("wuc"))
    if self.get_argument("to",default=None) is not None:
      query['to']=int(self.get_argument("wuc"))
    t2 = time.time()
    if len(query)>0:
      i1 = time.time()
      table = list(mongoDBClient.wukong.readingssystem.find(query))#device
      i2 = time.time()
      for it in table:
        tmp_request={"node_id":it['nodeId'],"port":it['portNumber']}
        np.append(tmp_request)
        #print "@LOOP"
        #print tmp_request
      i3 = time.time()
      #self.render('templates/value.html', applications=[Test_doc(np).result])
      self.write(json.dumps(Test_doc(np).result))
      i4 = time.time()
      print "[In][TIME] it take %f s Finding (node/port) @ system table" %(i2-i1)
      print "[In][TIME] it take %f s for matched result" %(i3-i2)
      print "[In][TIME] it take %f s Query Raw Data's table" %(i4-i3)
      t3 = time.time()
      print "[Out][TIME] it take %f s parsing URL" %(t2-t1)
      print "[Out][TIME] it take %f s for ALL DB's query" %(t3-t2)
    elif len(query)==0:
      if self.get_argument("node",default=None) is not None:
        np[0]['node_id']=int(self.get_argument("node"))
      if self.get_argument("port",default=None) is not None:
        np[0]['port']=int(self.get_argument("port")) 
      #self.render('templates/value.html', applications=[Test_doc(np).result])
      self.write(json.dumps(Test_doc(np).result))
      #http://localhost:8888/getvalue?arg1=IR%20Sensor&arg2=3&arg3=2&arg4=BL-7F%20entrance

class _CreateUser(tornado.web.RequestHandler):
  def get(self):
    query = {}
    np = []
    timerange = {}

    t1 = time.time()
    if self.get_argument("id",default=None) is not None:
      query['id']=str(self.get_argument("id"))
    else :
      self.write("Please provide UserID at URL")
    if self.get_argument("pwd",default=None) is not None:
      query['pwd']=str(self.get_argument("pwd"))
    else :
      self.write("Please provide UserPassword at URL")
    if self.get_argument("auth",default=None) is not None:
      query['auth']=str(self.get_argument("auth"))
    else :
      self.write("default auth is normal user")
    
    t2 = time.time()
    if self.get_argument("id",default=None) is not None and self.get_argument("pwd",default=None) is not None:
      cursor=mongoDBClient.wukong.user.find({"UserId":query['id'],"UserPassword":query['pwd']})

      print (list(cursor))

      if cursor.count()==0 : 
        mongoDBClient.wukong.user.insert({"UserId":query['id'],"UserPassword":query['pwd']})
        self.write("User EXIST:"+str(cursor.count()))
      else:
        self.write("User EXIST:"+str(cursor.count()))
      
      #self.write(findID)
  #http://localhost:8888/createuser?id=a&pwd=a

class CreateUser(tornado.web.RequestHandler):
  def get(self):
    query = {}


    t1 = time.time()
    if self.get_argument("id",default=None) is not None:
      query['id']=str(self.get_argument("id"))
    else :
      self.write("Please provide UserID at URL. ")
    if self.get_argument("pwd",default=None) is not None:
      query['pwd']=str(self.get_argument("pwd"))
    else :
      self.write("Please provide UserID at URL. ")
    if self.get_argument("type",default=None) is not None:
      query['type']=str(self.get_argument("type"))
    else :
      self.write("Please provide UserType at URL. ")
    if self.get_argument("pref",default=None) is not None:
      query['pref']=str(self.get_argument("pref"))
    else :
      self.write("Please provide UserPref at URL. ")
    if self.get_argument("loc",default=None) is not None:
      query['loc']=str(self.get_argument("loc"))
    else :
      self.write("Please provide UserLocation at URL. ")
    
    t2 = time.time()
    if self.get_argument("id",default=None) is not None and self.get_argument("pwd",default=None) is not None and self.get_argument("type",default=None) is not None and self.get_argument("pref",default=None) is not None and self.get_argument("loc",default=None) is not None:
      document={"User_id":query['id'],"User_pwd":query['pwd'],"Type":query['type'],"Preference":query['pref'],"Location":query['loc']}
      cursor=mongoDBClient.wukong.user.find(document)

      print (list(cursor))

      if cursor.count()==0 : 
        mongoDBClient.wukong.user.insert(document)
        self.write("Document EXIST:"+str(cursor.count()))
      else:
        self.write("Document EXIST:"+str(cursor.count()))
      
      #self.write(findID)
  #http://localhost:8888/createuser?id=wukong&pwd=wukong2014&type=adult&pref=Null&loc=BL-7F/Workspace/Entrance

class CreateSystem(tornado.web.RequestHandler):
  def get(self):
    query = {}


    t1 = time.time()
    if self.get_argument("id",default=None) is not None:
      query['id']=str(self.get_argument("id"))
    else :
      self.write("Please provide UserID at URL")
    if self.get_argument("holder",default=None) is not None:
      query['holder']=str(self.get_argument("holder"))
    else :
      self.write("Argument missed")
    if self.get_argument("devices",default=None) is not None:
      query['devices']=str(self.get_argument("devices"))
    else :
      self.write("Argument missed")
    if self.get_argument("fbps",default=None) is not None:
      query['fbps']=str(self.get_argument("fbps"))
    else :
      self.write("Argument missed")
    if self.get_argument("gateways",default=None) is not None:
      query['gateways']=str(self.get_argument("gateways"))
    else :
      self.write("Argument missed")
    if self.get_argument("wuclasses",default=None) is not None:
      query['wuclasses']=str(self.get_argument("wuclasses"))
    else :
      self.write("Argument missed")
    
    t2 = time.time()
    if self.get_argument("id",default=None) is not None and self.get_argument("holder",default=None) is not None and self.get_argument("devices",default=None) is not None and self.get_argument("fbps",default=None) is not None and self.get_argument("gateways",default=None) is not None and self.get_argument("wuclasses",default=None) is not None:
      document={"Project_id":query['id'],"House_holder":query['holder'],"Device_list":query['devices'],"FBP_list":query['fbps'],"Gateway_list":query['gateways'],"Wuclass_list":query['wuclasses']}
      cursor=mongoDBClient.wukong.system.find(document)

      print (list(cursor))

      if cursor.count()==0 : 
        mongoDBClient.wukong.system.insert(document)
        self.write("Document EXIST:"+str(cursor.count()))
      else:
        self.write("Document EXIST:"+str(cursor.count()))
      
      #self.write(findID)
  #http://localhost:8888/createsystem?id=a&holder=b&devices=c&fbps=d&gateways=e&wuclasses=f
class CreateDevice(tornado.web.RequestHandler):
  def get(self):
    query = {}



    t1 = time.time()
    if self.get_argument("id",default=None) is not None:
      query['id']=str(self.get_argument("id"))
    else :
      self.write("Argument missed1")
    if self.get_argument("wuobject",default=None) is not None:
      query['wuobject']=str(self.get_argument("wuobject"))
    else :
      self.write("Argument missed2")
    if self.get_argument("type",default=None) is not None:
      query['type']=str(self.get_argument("type"))
    else :
      self.write("Argument missed3")
    if self.get_argument("capacity",default=None) is not None:
      query['capacity']=str(self.get_argument("capacity"))
    else :
      self.write("Argument missed4")
    if self.get_argument("network",default=None) is not None:
      query['network']=str(self.get_argument("network"))
    else :
      self.write("Argument missed5")
    if self.get_argument("loc",default=None) is not None:
      query['loc']=str(self.get_argument("loc"))
    else :
      self.write("Argument missed6")

    
    t2 = time.time()
    if self.get_argument("id",default=None) is not None and self.get_argument("wuobject",default=None) is not None and self.get_argument("type",default=None) is not None and self.get_argument("capacity",default=None) is not None and self.get_argument("network",default=None) is not None and self.get_argument("loc",default=None) is not None:
      document={"Device_id":query['id'],"WuObject":query['wuobject'],"Type":query['type'],"Capacity":query['capacity'],"Network":query['network'],"Location":query['loc']}
      cursor=mongoDBClient.wukong.device.find(document)

      print (list(cursor))

      if cursor.count()==0 : 
        mongoDBClient.wukong.device.insert(document)
        self.write("Document EXIST:"+str(cursor.count()))
      else:
        self.write("Document EXIST:"+str(cursor.count()))
      
      #self.write(findID)
  #http://localhost:8888/createdevice?id=a&wuobject=b&type=c&capacity=d&network=e&loc=f
class DeleteDevice(tornado.web.RequestHandler):
  def get(self):
    query = {}


    self.write("DeleteDevice: ")
    t1 = time.time()
    if self.get_argument("Did",default=None) is not None:
      query['Did']=str(self.get_argument("Did"))
      self.write(query['Did'])
    else :
      self.write("Argument missed1")
    

    
    t2 = time.time()
    if self.get_argument("Did",default=None) is not None :
      document={"_id":ObjectId(query['Did'])}
      print document
      self.write(json.dumps(   JSONEncoder().encode(Print_doc(document,"device").result)  ))
      #result=mongoDBClient.wukong.device.delete_many(document)
      #print result.deleted_count
      result=mongoDBClient.wukong.device.remove({"_id":ObjectId(query['Did'])})#55405bd3491d436cb5288911
      print result
      print "Delete Complete"



      
      #self.write(findID)
  #http://localhost:8888/deletedevice?Did=55405bd3491d436cb5288911

class UpdateDeviceByWuobject(tornado.web.RequestHandler):
  def get(self):
    query = {}


    self.write("UpdateDeviceByWuobject: ")
    t1 = time.time()
    if self.get_argument("Did",default=None) is not None:
      query['Did']=str(self.get_argument("Did"))
      self.write(query['Did'])
    else :
      self.write("Argument missed1")
    if self.get_argument("wuobject",default=None) is not None:
      query['wuobject']=str(self.get_argument("wuobject"))
      self.write(query['wuobject'])
    else :
      self.write("Argument missed2")

    
    t2 = time.time()
    if self.get_argument("Did",default=None) is not None and self.get_argument("wuobject",default=None) is not None:

      result=mongoDBClient.wukong.device.update({"_id":ObjectId(query['Did'])},{"$set":{"WuObject":query['wuobject']}},upsert=False)#55405bd3491d436cb5288911
      print result
      print "Delete Complete"



      
      #self.write(findID)
  #http://localhost:8888/deletedevice?Did=55405bd3491d436cb5288911
class FindDevice(tornado.web.RequestHandler):
  def get(self):

    print "FindDevice"
    '''
    for it in mongoDBClient.wukong.device.find({}):
      print it 
      it['_id']=str(it['_id'])
      self.write(json.dumps(it))
    '''
    #print Print_doc({},"device").result
    self.write(json.dumps(   JSONEncoder().encode(Print_doc({},"device").result)  ))
    #self.write(json.dumps(   JSONEncoder().encode(Print_doc({},"test").result)  ))#test

      
      #self.write(findID)
  #http://localhost:8888/createdevice?id=a&wuobject=b&type=c&capacity=d&network=e&loc=f

application = tornado.web.Application([
    (r"/", MainHandler),
    (r"/getvalue",GetValue),
    (r"/_createuser",_CreateUser),
    (r"/createuser",CreateUser),
    (r"/createsystem",CreateSystem),
    (r"/createdevice",CreateDevice),
    (r"/deletedevice",DeleteDevice),
    (r"/updatedevicebywuobj",UpdateDeviceByWuobject),
    (r"/finddevice",FindDevice),
    (r"/asd",asd),
    (r"/get",Get )
])

if __name__ == "__main__":
    application.listen(8888)
    tornado.ioloop.IOLoop.instance().start()