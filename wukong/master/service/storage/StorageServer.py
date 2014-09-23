from protobuf.socketrpc import *

@Singleton
class StorageServer(object):
  started = false;

  def __init__(self, port):
    self.port = port;

  def start(self):
    if (!started):
      server = server.SocketRpcServer(self.port)
      server.registerService();
      server.run()
      started = true