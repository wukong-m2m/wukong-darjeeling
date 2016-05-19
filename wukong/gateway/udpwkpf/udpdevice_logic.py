from udpwkpf import WuClass, Device
import sys
from twisted.protocols import basic
from twisted.internet import reactor, protocol
from twisted.web import resource,static,server
import traceback

global site

if __name__ == "__main__":
    class UIButton(WuClass):
        def __init__(self):
            WuClass.__init__(self)
            self.loadClass('UIButton')
        def update(self,obj,pID=None,val=None):
            pass
        def init(self):
            pass

    class Threshold(WuClass):
        def __init__(self):
            WuClass.__init__(self)
            self.loadClass('Threshold')
        def update(self,obj,pID,val):
            if pID == 2:
                op = obj.getProperty(0)
                if op == 0:
                    if val < obj.getProperty(1):
                        obj.setProperty(3,True)
                        print "value %d < threshold %d: True" % (val, obj.getProperty(1))
                    else:
                        obj.setProperty(3,False)
                        print "value %d < threshold %d: False" % (val, obj.getProperty(1))
                elif op == 1:
                    if val > obj.getProperty(1):
                        obj.setProperty(3,True)
                        print "value %d > threshold %d: True" % (val, obj.getProperty(1))
                    else:
                        obj.setProperty(3,False)
                        print "value %d > threshold %d: False" % (val, obj.getProperty(1))
                elif op == 2:
                    if val <= obj.getProperty(1):
                        obj.setProperty(3,True)
                        print "value %d <= threshold %d: True" % (val, obj.getProperty(1))
                    else:
                        obj.setProperty(3,False)
                        print "value %d <= threshold %d: False" % (val, obj.getProperty(1))
                elif op == 3:
                    if val >= obj.getProperty(1):
                        obj.setProperty(3,True)
                        print "value %d >= threshold %d: True" % (val, obj.getProperty(1))
                    else:
                        obj.setProperty(3,False)
                        print "value %d >= threshold %d: False" % (val, obj.getProperty(1))
                else:
                    print "Error: unknown operator %d" % op

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

    class Click(resource.Resource):
        isLeaf = True
        def __init__(self,device):
            self.device = device
            self.UIButton_ID = 11002
        def render_GET(self,request):
            try:
                n = (int)(request.args.get('click')[0])
                n = True if n > 0 else False
                print n
                print self.device.objects
                for i in range(0,len(self.device.objects)):
                    obj = self.device.objects[i]
                    print obj
                    if obj.getID() == self.UIButton_ID:
                        try:
                            #obj.setProperty(obj.cls.number,(n))
                            obj.setProperty(0,n)
                        except:
                            traceback.print_exc()
                        break
            except:
                pass
            return '%s' % (n)

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)
        def init(self):
            m1 = Threshold()
            self.addClass(m1,self.FLAG_VIRTUAL)
            self.addObject(m1.ID)
            m2 = UIButton()
            self.addClass(m2,self.FLAG_APP_CAN_CREATE_INSTANCE| self.FLAG_VIRTUAL)

    if len(sys.argv) <= 2:
        print 'python %s <gip> <dip>:<port>' % sys.argv[0]
        print '      <gip>: IP addrees of gateway'
        print '      <dip>: IP address of Python device'
        print '      <port>: An unique port number'
        print ' ex. python %s 192.168.4.7 127.0.0.1:3000' % sys.argv[0]
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    root=static.File("./www")
    root.putChild("click",Click(d))
    root.putChild("show",ShowAll(d))
    site=server.Site(root)
    site.device = d
    reactor.listenTCP(11002,site)
    reactor.run()

