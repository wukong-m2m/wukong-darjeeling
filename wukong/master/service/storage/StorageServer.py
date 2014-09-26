from protobuf.socketrpc import server
from mongo.MongoDBStorageServiceImpl import *

class StorageServer(object):
  started = False

  def __init__(self, port):
    self.port = port;

  def start(self):
    if (not StorageServer.started):
      storage_sever = server.SocketRpcServer(self.port)
      storage_sever.registerService(MongoDBStorageServiceImpl())
      storage_sever.run()
      StorageServer.started = True


def main():
    server = StorageServer(8080);
    server.start()


if  __name__ =='__main__':
    main()