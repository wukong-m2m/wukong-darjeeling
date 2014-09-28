from protobuf.socketrpc import RpcService
from model.storage_pb2 import *
import time
import logging
log = logging.getLogger(__name__)
logging.basicConfig(level=logging.DEBUG)

class StorageService(object):

  def __init__(self, host, port):
    self.service = RpcService(StorageService_Stub, port, host)

  def storeSensorData(self, sensordata, callback):
    try:
        self.service.storeSensorData(sensordata, callback=callback)
    except Exception, ex:
        log.exception(ex)

  def storeContextData(self, contextdata, callback):
    try:
        self.service.storeContextData(contextdata, callback=callback)
    except Exception, ex:
        log.exception(ex)

  def getSensorData(self, fetchrequest, callback):
    try:
        fetchresposne = self.service.fetchSensorData(fetchrequest, callback=callback)
    except Exception, ex:
        log.exception(ex)

class Callback():
    def run(self, response):
        print "Received Response: %s" % response

def main():
    service = StorageService('localhost', 8888)
    sensordata = SensorData()
    sensordata.type = SensorData.PRESSURE_SENSOR
    sensordata.timestamp = 123456
    sensordata.value = 3
    sensordata.location.type = Location.BEDROOM

    while True:
        response = service.storeSensorData(sensordata, Callback())
        time.sleep(1)

if  __name__ =='__main__':
    main()