from model.storage_pb2 import *

class HbaseStorageServiceImpl(StorageService):
  def __init__(self):
     """Initialize Hbase Connection """

  def storeSensorData(self, controller, sensordata, callback):
    """Received sensor data """
    print "storeSensorData received data from client"
    callback.run(void())

  def storeContextData(self, controller, sensordata, callback):
    print "storeContextData received data from client"
    callback.run(void())

  def getSensorData(self, controller, fetchrequest, callback):
    print "getSensorData received data from client"
    callback.run(void())

