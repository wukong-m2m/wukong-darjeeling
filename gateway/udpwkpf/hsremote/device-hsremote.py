from udpwkpf import WuClass,Device
from twisted.internet.protocol import DatagramProtocol
from twisted.internet import reactor
from twisted.web import resource,static,server
from twisted.internet.protocol import Factory,Protocol
from twisted.internet.endpoints import TCP4ServerEndpoint
import netifaces
import __builtin__
import cjson
import struct
global site

import sys
myinterface = "wlan0"

class UIButton(WuClass):
    def __init__(self):
        self.loadClass('UIButton')
    def udpate(self,obj,pID,val):
        pass
    def init(self):
        pass

class UISlider(WuClass):
    def __init__(self):
        self.loadClass('UISlider')
    def udpate(self,obj,pID,val):
        pass
    def init(self):
        pass
class UIScene(WuClass):
    def __init__(self):
        self.loadClass('UIScene')
    def udpate(self,obj,pID,val):
        pass
    def init(self):
        pass
class HsremoteDevice(Device):
    def __init__(self,gtwip,selfip):
        Device.__init__(self,gtwip,selfip)
    def init(self):
        print "xxxxxx"
        cls = UIButton()
        self.addClass(cls,self.FLAG_APP_CAN_CREATE_INSTANCE| self.FLAG_VIRTUAL)
        cls = UISlider()
        self.addClass(cls,self.FLAG_APP_CAN_CREATE_INSTANCE| self.FLAG_VIRTUAL)
        cls = UIScene()
        self.addClass(cls,self.FLAG_APP_CAN_CREATE_INSTANCE| self.FLAG_VIRTUAL)
        self.ID_slider = cls.getWuClassID('UISlider')
        self.ID_scene = cls.getWuClassID('UIScene')
        pass
    def update(self,obj,pID,val):
        pass
    def findSlider(self,p,d):
        for i in range(0,len(self.objects)):
            obj = self.objects[i]
            if obj.cls.ID == self.ID_slider:
                print obj.port,obj.getProperty(1),obj.getProperty(2)
                if obj.getProperty(1) == p and obj.getProperty(2) == d:
                    return obj
        return None
    def findScene(self,p,d):
        for i in range(0,len(self.objects)):
            obj = self.objects[i]
            if obj.cls.ID == self.ID_scene:
                if obj.getProperty(1) == p and obj.getProperty(2) == d:
                    return obj
        return None

class SearchProtocol(DatagramProtocol):
    def datagramReceived(self, data, (host, port)):
        # \x01\x00\x00\xc0\x00\x00\x00\x00\x00\x01\xff\xff
        net = netifaces.ifaddresses(myinterface)
        mac = net[netifaces.AF_LINK]
        ip = map(int,net[netifaces.AF_INET][0]['addr'].split('.'))
        print ip
        mac = netifaces.ifaddresses(myinterface)[netifaces.AF_LINK][0]['addr'].split(':')
        print mac
        
        header = struct.pack("hBBIhh6B",1,0,0xc0,0,1,2,int(mac[0],16),int(mac[1],16),int(mac[2],16),int(mac[3],16),int(mac[4],16),int(mac[5],16))
        data = header+struct.pack("4BII", ip[0],ip[1],ip[2],ip[3],0,0)+"wukong"+"\0"*14
        self.transport.write(data,(host,port))
        print map(ord, data)
        print "receive data %r from %s:%d" % (data,host,port)
class Cgi(Protocol):
    isLeaf = True
    def __init__(self,factory):
        self.factory = factory
    def dataReceived(self,request):
        print request
        # /cgi-bin/proxy?cmd=/scene&p=1&d=113&serialno=,65535
        # GET /cgi-bin/proxy?cmd=/scene&p=1&d=113&serialno=,65535 HTTP/1.
        lines = request.split('\n')
        print lines[0]
        tokens = lines[0].split(' ')
        ulist = tokens[1].split('?')
        if len(ulist) >= 2:
            paths = ulist[0].split('/')
            pars = ulist[1].split('&')
            query = {}
            for p in pars:
                ff = p.split('=')
                if len(ff) != 2:
                    return self.render_none()
                query[ff[0]] = ff[1]
        else:
            paths = ulist[0].split('/')
            query={}
        print paths
        if tokens[0] == 'GET':
            if paths[1] == 'cgi-bin':
                if paths[2] == 'proxy':
                    print query
                    if query['cmd'] == '/scene':
                        self.render_scene(query)
                    elif query['cmd'][0:2] == '/r':
                        self.render_control(query['cmd'])
                    else:
                        self.render_none()
                else:
                    self.render_none()
            else:
                self.render_none()
        elif tokens[0] == 'POST':
            print 'POST method',paths
            if paths[1] == 'cgi-bin':
                if paths[2] == 'ssidlist':
                    print lines
                    for i in range(len(lines)):
                        if lines[i] == '' or lines[i] == '\r':
                            try:
                                obj = cjson.decode('\n'.join(lines[i+1:]))
                                print obj
                            except:
                                pass
            self.render_none()

    def doCommand_clear(self):
        if self.clear_obj:
            self.clear_obj.setProperty(0,7)
            self.clear_obj.setProperty(3,False)
            self.clear_obj = None
    def doCommand(self,d,p,v):
        obj = self.factory.device.findSlider(d,p)
        if obj:
            if v == 30: #up
                obj.setProperty(0,3)
            elif v == 31: #down
                obj.setProperty(0,4)
            elif v == 32: #left
                obj.setProperty(0,1)
            elif v == 33: #right
                obj.setProperty(0,2)
            elif v == 19: #O
                obj.setProperty(0,5)
            elif v == 21: #X
                obj.setProperty(0,6)
            elif v == 20: #Stop
                obj.setProperty(0,0)
            elif v == 39: #change mode
                obj.setProperty(3,True)
            if d == 11 or d == 12 or d == 13 or  d == 14:
                print "CMD: %d reset" % (d)
                self.clear_obj  = obj
                reactor.callLater(0.5, self.doCommand_clear)
    def doScene(self,s):
        obj = self.factory.device.findScene(1,s)
        if obj: obj.setProperty(0,v)
    def render_control(self,cmd):
        cmd = cmd[2:].split(',')
        print ' command %0x' % int(cmd[0])
        arg = int(cmd[0])
        v = (arg >> 16)&0xff
        d = (arg>>8)&0xff
        p = arg & 0xff
        print "CMD (%d,%d) = %d" % (d,p,v)
        self.doCommand(d,p,v)
        self.transport.write('HTTP/1.1 200 OK\nContent-Length: 0\n\n')
        self.transport.loseConnection()

    def render_scene(self,query):
        d = int(query['d'])
        p = int(query['p'])
        print 'render_scene'
        if p == 1 and (d>=113 & d <= 117):
            s = d-113+1
            print 'Do scene %d' % s
            self.doScene(s)
        self.transport.write('HTTP/1.1 200 OK\nContent-Length: 0\n\n')
        self.transport.loseConnection()

    def render_none(self):
        self.transport.write('HTTP/1.1 200 OK\nContent-Length: 1\n\n0')
        self.transport.loseConnection()
class CgiFactory(Factory):
    def __init__(self,device):
        self.device = device
    def buildProtocol(self, addr):
        return Cgi(self)

# if "__name__" == "__main__":
d = HsremoteDevice(sys.argv[1], sys.argv[2])
root=static.File("./www")
root.putChild("cgi-bin",Cgi(d))
site=server.Site(root)
site.device = d
endpoint = TCP4ServerEndpoint(reactor, 80)
endpoint.listen(CgiFactory(d))
reactor.listenUDP(8000, SearchProtocol())
print "HsremoteDevice is ready"
reactor.run()
