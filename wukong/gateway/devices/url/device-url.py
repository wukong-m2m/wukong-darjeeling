from udpwkpf import WuClass,Device
from twisted.internet import reactor
from twisted.web import resource,static,server
from autobahn.twisted.websocket import WebSocketServerFactory,WebSocketServerProtocol
from autobahn.twisted.resource import WebSocketResource,HTTPChannelHixie76Aware
import __builtin__
import cjson
global site

class URL(WuClass):
    PICTURE=0
    def __init__(self):
        self.ID = self.getWuClassID('URL')
    def udpate(self):
        print "receive URL"
        pass
    def init(self):
        pass

class URLDevice(Device):
    def __init__(self):
        Device.__init__(self,'127.0.0.1','127.0.0.1:4001')
    def init(self):
        print "xxxxxx"
        cls = URL()
        self.addClass(cls)
        self.obj_url = self.addObject(cls.ID)
        pass
    def update(self,obj,pID,val):
        pass
class Click(resource.Resource):
    isLeaf = True
    def __init__(self,device):
        self.device = device
    def render_GET(self,request):
        self.device.obj_url.setProperty(0,self.device.obj_url.getProperty(0)+1)
        return '%d' % self.device.obj_url.getProperty(0)

d = URLDevice()
root=static.File("./www")
root.putChild("click",Click(d))
site=server.Site(root)
site.device = d
reactor.listenTCP(11005,site)

__builtin__.site = site
reactor.run()
