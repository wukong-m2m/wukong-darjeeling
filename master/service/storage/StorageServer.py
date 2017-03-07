from protobuf.socketrpc import server
from mongo.MongoDBStorageServiceImpl import *
from model.storage_pb2 import *

class StorageServer(object):
  started = False

  def __init__(self, port):
    self.port = port;

  def start(self):
    if not StorageServer.started:
      StorageServer.started = True
      storage_sever = server.SocketRpcServer(self.port)
      storage_sever.registerService(MongoDBStorageServiceImpl())
      print 'Start storage server on port', self.port
      storage_sever.run()


def main():
    server = StorageServer(8888);
    server.start()

if  __name__ =='__main__':
    main()