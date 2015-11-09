from udpwkpf import WuClass,Device
from twisted.internet import reactor
from twisted.web import resource,static,server
from autobahn.twisted.websocket import WebSocketServerFactory,WebSocketServerProtocol
from autobahn.twisted.resource import WebSocketResource,HTTPChannelHixie76Aware
import __builtin__
import cjson
global site

class UIImage(WuClass):
    PICTURE=0
    def __init__(self):
        self.ID = 11001
    def getPicture(self,obj):
        return self.wkpf.properties[obj.port][0]
class RFID(WuClass):
    def __init__(self):
        self.ID=10001
    def update(self,obj,pID,val):
        pass
class picture(WuClass):
    ID=0
    PICTURE=1
    def __init__(self):
        self.ID=10002
    def update(self,obj,pID,val):
        pass
    def NextPicture(self,obj):
        print 'next picture'
        self.setProperty(obj.port,1,obj.getProperty(picture.PICTURE)+1)

class EMAP(Device):
    def __init__(self):
        Device.__init__(self,'127.0.0.1','127.0.0.1:4000')
    def update(self,obj,pID,val):
        msg = cjson.encode({'ID':self.page_ID,'src':self.obj_image.cls.getPicture(self.obj_image)})
        self.page_protocol.sendMessage(cjson.encode(msg))
    def init(self):
        cls = UIImage()
        self.addClass(cls)
        self.obj_image = self.addObject(cls.ID)
        cls = RFID()
        self.addClass(cls)
        self.obj_rfid = self.addObject(cls.ID)
        cls = picture()
        self.addClass(cls)
        self.obj_picture = self.addObject(cls.ID)
        self.page_ID = ''
    def addImageUI(self,protocol,ID):
        self.page_ID = ID
        self.page_protocol = protocol
    def getPicture(self):
        pic = self.obj_image.getProperty(UIImage.PICTURE)
        print 'Picture:', pic
        return pic
class EMAPServerProtocol(WebSocketServerProtocol):
    def onConnect(self,request):
        pass
    def onMessage(self,payload,isBinary):
        print payload
        p = cjson.decode(payload)
        if p['cmd'] == 'get':
            self.updateWuObjects(p['objects'])
        elif p['cmd'] == 'next':
            site.device.obj_picture.cls.NextPicture(site.device.obj_picture)
        pass
    def updateWuObjects(self,objects):
        for o in objects:
            if o['type'] == 'image':
                site.device.addImageUI(self,o['id'])
                reactor.callLater(1,self.update)
    def update(self,obj,pID,val):
        self.sendMessage('%d' % site.device.getPicture(),False)
        reactor.callLater(1,self.update)


d = EMAP()
factory = WebSocketServerFactory("ws://localhost:8888",debug = True,debugCodePaths = True)
factory.protocol = EMAPServerProtocol
factory.setProtocolOptions(allowHixie76 = True)
resource = WebSocketResource(factory)
root=static.File("./emap")
root.putChild("ws", resource)
site=server.Site(root)
site.protocol = HTTPChannelHixie76Aware
site.device = d
reactor.listenTCP(8888,site)
__builtin__.site = site
reactor.run()
