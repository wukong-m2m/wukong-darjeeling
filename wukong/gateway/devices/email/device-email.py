from udpwkpf import WuClass,Device
from twisted.internet import reactor
from twisted.web import resource,static,server
from autobahn.twisted.websocket import WebSocketServerFactory,WebSocketServerProtocol
from autobahn.twisted.resource import WebSocketResource,HTTPChannelHixie76Aware
import __builtin__
import cjson
global site

class EMail(WuClass):
    PICTURE=0
    def __init__(self):
        self.ID = 11004
    def udpate(self):
        print "send email" 
        pass
    def init(self):
        pass
class EMailDevice(Device):
    def __init__(self):
        Device.__init__(self,'127.0.0.1','127.0.0.1:4002')
    def init(self):
        cls = EMail()
        self.addClass(cls)
        self.obj = self.addObject(cls.ID)
    def update(self):
        print "update"

d = EMailDevice()
root=static.File("./www")
site=server.Site(root)
site.device = d
reactor.listenTCP(11004,site)
__builtin__.site = site
reactor.run()
