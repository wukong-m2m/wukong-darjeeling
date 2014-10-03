from model.storage_pb2 import *
import pymongo
from pymongo import MongoClient
import datetime
from bson import BSON

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
    self.collection = self.db["readings"]
    print 'Create mongoDB connection'

  def storeSensorData(self, controller, sensordata, callback):
    """Received sensor data """
    self.post_sensor = {"id": "sensor_%s" %(sensordata.SensorType),
    "data" : sensordata.SerializeToString()
    }
    self.post_id_s = self.db.posts.insert(self.post_sensor)
    print self.db.posts.find_one(self.post_id_s)

    callback.run(void())

  def storeContextData(self, controller, sensordata, callback):
    print "received data from client"
    self.post_context = {"id": "context_%s" %(sensordata.SensorType),
    "data" : sensordata.SerializeToString()
    }
    self.post_id_s = self.db.posts.insert(self.post_sensor)
    print self.db.posts.find_one(self.post_id_s)
    callback.run(void())

  '''Mongo API'''  
  def retrieve_sensor(self,n_id,pt):
        for p in self.db.readings.find({ 'node_id':n_id , 'port':pt }) :
            print '[Specific Sensor]+%s'%(p)

  def retrieve_time(self,t1,t2):
        for p in self.db.readings.find( { 'timestamp':  { "$gte":t1 , "$lt": t2 } } ) :
            print '[Specific time]+%s'%(p)


  def getSensorData(self, controller, fetchrequest, callback):
    print "received data from client"
    my_fetchrequest=storage_pb2.FetchRequest()
    my_fetchrequest= fetchrequest
    '''print location/start_timestamp/end_timestamp'''
    print "retrieve condition location/start_timestamp/end_timestamp =  %s/%d/%d" %(my_fetchrequest.location,my_fetchrequest.start_timestamp,my_fetchrequest.end_timestamp)
    '''Here we got no location schema from sensor, ONLY node_id&port'''
    retrieve_time(my_fetchrequest.start_timestamp,my_fetchrequest.end_timestamp)


    callback.run(void())


st= MongoDBStorageServiceImpl()
