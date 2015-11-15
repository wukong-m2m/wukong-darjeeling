from udpwkpf import WuClass,Device
from twisted.internet import reactor
from twisted.web import resource,static,server
from autobahn.twisted.websocket import WebSocketServerFactory,WebSocketServerProtocol
from autobahn.twisted.resource import WebSocketResource,HTTPChannelHixie76Aware
import __builtin__
import cjson,traceback
global site

class UISlider(WuClass):
    def __init__(self):
        self.loadClass('UISlider')
    def udpate(self,obj,pID,val):
        pass
    def init(self):
        pass

class Threshold(WuClass):
    def __init__(self):
        self.loadClass('Threshold')
    def udpate(self,obj,pID,val):
        if pID == self.value:
            op = obj.getProperty(self.operator)
            if op == 0:
                if val < obj.getProperty(self.threshold):
                    obj.setProperty(self.output,true)
                else:
                    obj.setProperty(self.output,false)
            elif op == 1:
                if val > obj.getProperty(self.threshold):
                    obj.setProperty(self.output,true)
                else:
                    obj.setProperty(self.output,false)
            elif op == 2:
                if val <= obj.getProperty(self.threshold):
                    obj.setProperty(self.output,true)
                else:
                    obj.setProperty(self.output,false)
            elif op == 3:
                if val >= obj.getProperty(self.threshold):
                    obj.setProperty(self.output,true)
                else:
                    obj.setProperty(self.output,false)
            else:
                print "Error: unknown operator %d" % op
        pass
    def init(self):
        pass

class LogicDevice(Device):
    def __init__(self):
        Device.__init__(self,'127.0.0.1','127.0.0.1:4001')
    def init(self):
        cls = Threshold()
        self.addClass(cls,self.FLAG_APP_CAN_CREATE_INSTANCE| self.FLAG_VIRTUAL)
        cls = UISlider()
        self.addClass(cls,self.FLAG_APP_CAN_CREATE_INSTANCE| self.FLAG_VIRTUAL)
        pass
    def update(self,obj,pID,val):
        pass

class ShowAll(resource.Resource):
    isLeaf = True
    def __init__(self,device):
        self.device = device
    def render_GET(self,request):
        s = '<html><pre>\n'
        for i in range(0,len(self.device.objects)):
            obj = self.device.objects[i]
            s = s + 'Object(port = %d)\n' % obj.port
            for j in range(0,obj.cls.getPropertyNumber()):
                s = s + '    %s --> %d\n' %( obj.cls.getPropertyName(j), obj.getProperty(j))
        s = s + '</pre></html>\n'
        return s.encode('utf-8')

class Number(resource.Resource):
    isLeaf = True
    def __init__(self,device):
        self.device = device
        self.sliderID = 11006
    def render_GET(self,request):
        try:
            n = request.args.get('number')[0]
            print self.device.objects
            for i in range(0,len(self.device.objects)):
                obj = self.device.objects[i]
                print obj
                if obj.getID() == self.sliderID:
                    print 'xxxxx'
                    try:
                        obj.setProperty(obj.cls.number,int(n))
                    except:
                        traceback.print_exc()
                    break
        except:
            pass
        return '%d' % int(n)
d = LogicDevice()
root=static.File("./www")
root.putChild("slider",Number(d))
root.putChild("show",ShowAll(d))
site=server.Site(root)
site.device = d
reactor.listenTCP(11006,site)
reactor.run()
