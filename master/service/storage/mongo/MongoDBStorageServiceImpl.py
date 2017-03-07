
import os, sys
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../..'))
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from model.storage_pb2 import *
import pymongo
from pymongo import MongoClient
import datetime
from bson import BSON
from bson.binary import Binary, UUIDLegacy, UUID_SUBTYPE

'''
class Callback():
    def run(self):
        print ""
'''

class MongoDBStorageServiceImpl(StorageService):
  def __init__(self):
    self.ip="140.112.170.32"
    self.client=None
    self.db=None
    self.collection=None
    self.post_sensor=None
    self.post_context=None
    self.client = MongoClient(self.ip, 27017)
    self.db = self.client["wukong"]
    self.collection = self.db["posts"]
    print 'Create mongoDB connection'

  def storeSensorData(self, controller, sensordata, callback):
    self.post_sensor = {"id": "sensor_%s" %(sensordata.type),
    "timestamp" : sensordata.timestamp ,
    "data" : Binary(sensordata.SerializeToString())
    }
    self.post_id_s = self.db.posts.insert(self.post_sensor)
    print self.db.posts.find_one(self.post_id_s)

    #callback.run(void())

  def storeContextData(self, controller, sensordata, callback):
    self.post_context = {"id": "context_%s" %(sensordata.SensorType),
    "timestamp" : sensordata.timestamp ,
    "data" : Binary(sensordata.SerializeToString())
    }
    self.post_id_s = self.db.posts.insert(self.post_context)
    return self.post_id_s
    #print self.db.posts.find_one(self.post_id_s)
    #callback.run(void())

  

  '''Mongo API'''  
  def retrieve_sensor(self,n_id,pt):
        return self.db.posts.find({ 'node_id':n_id , 'port':pt })
        for p in self.db.posts.find({ 'node_id':n_id , 'port':pt }) :
            print '[Specific Sensor]+%s'%(p)

  def retrieve_time(self,t1,t2):
        return self.db.posts.find( { 'timestamp':  { "$gte":t1 , "$lt": t2 } } ) 
        for p in self.db.posts.find( { 'timestamp':  { "$gte":t1 , "$lt": t2 } } ) :
            print '[Specific time]+%s'%(p)
  def retrieve_st(self,n_id,pt,t1,t2):
        return self.db.posts.find( { 'node_id':n_id , 'port':pt ,'timestamp':  { "$gte":t1 , "$lt": t2 } } ) 
        for p in self.db.posts.find( { 'node_id':n_id , 'port':pt ,'timestamp':  { "$gte":t1 , "$lt": t2 } } ) :
            print '[Specific time]+%s'%(p)

  def getSensorData(self, controller, fetchrequest, callback):
    print "received data from client"
    my_fetchrequest=FetchRequest()
    my_fetchrequest= fetchrequest
    return self.retrieve_time(my_fetchrequest.start_timestamp,my_fetchrequest.end_timestamp)


    #callback.run(void())


st= MongoDBStorageServiceImpl()
sensordata = SensorData()
sensordata.type = SensorData.PRESSURE_SENSOR
sensordata.timestamp = 123456
sensordata.value = 19
sensordata.location.type = Location.BEDROOM
#st.storeSensorData("controller",sensordata,"Callback")
insert_id=st.storeContextData("controller",sensordata,"Callback")
fetchrequest = FetchRequest()
fetchrequest.start_timestamp=123456
fetchrequest.end_timestamp=123457
ans=st.getSensorData("controller",fetchrequest,"Callback")
print 'There are %d data' %(ans.count()) 

for a in ans:
  if insert_id==a["_id"]:
    print "got it!"
