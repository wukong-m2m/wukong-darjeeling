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
            self.loadClass('UIButton')
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
            self.addClass(m1,self.FLAG_APP_CAN_CREATE_INSTANCE| self.FLAG_VIRTUAL)
            m2 = UIButton()
            self.addClass(m2,self.FLAG_APP_CAN_CREATE_INSTANCE| self.FLAG_VIRTUAL)

    if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python <filename> <gateway ip> <local ip>:<any given port number>'
        print ' ex. python udpdevice_eeg_server.py 192.168.4.7 127.0.0.1:3000'
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    root=static.File("./www")
    root.putChild("click",Click(d))
    root.putChild("show",ShowAll(d))
    site=server.Site(root)
    site.device = d
    reactor.listenTCP(11002,site)
    reactor.run()

