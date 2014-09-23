from model.proto.storage_pb2 import *
from protobuf.socketrpc import *

@Singleton
class StorageService(object):
  def __init__(self):
    self.channel = SocketRpcChannel(host, port)
    self.controller = channel.newController()
    self.service = StorageService_Stub(channel)

  def storeSensorData(self, sensordata):
    self.service.storeSensorData(self.controller, sensordata)
    if (controller.failed()):
      print "Rpc failed %s : %s" % (controller.error, controller.reason)

  def storeContextData(self, contextdata):
    self.service.storeContextData(self.controller, contextdata)
    if (controller.failed()):
      print "Rpc failed %s : %s" % (controller.error, controller.reason)

  def getSensorData(self, fetchrequest):
    fetchresposne = self.service.fetchSensorData(self.controller, fetchrequest)
    if (controller.failed()):
      print "Rpc failed %s : %s" % (controller.error, controller.reason)
    return fetchresponse