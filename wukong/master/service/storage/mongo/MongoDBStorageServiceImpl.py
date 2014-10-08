import os, sys
lib_path = os.path.abspath('..')
sys.path.append(lib_path)

from model.storage_pb2 import *

class MongoDBStorageServiceImpl(StorageService):
  def __init__(self):
     """Initialize MongoDB Connection """
     print 'Create mongoDB connection'

  def storeSensorData(self, controller, sensordata, callback):
    """Received sensor data """
    print "received data from client"
    callback.run(void())

  def storeContextData(self, controller, sensordata, callback):
    print "received data from client"
    callback.run(void())

  def getSensorData(self, controller, fetchrequest, callback):
    print "received data from client"
    callback.run(void())

def main():
   service = MongoDBStorageServiceImpl();

if  __name__ =='__main__':
    main()