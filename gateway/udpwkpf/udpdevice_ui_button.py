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
        def udpate(self,obj,pID,val):
            pass
        def init(self):
            pass

    class Click(resource.Resource):
        isLeaf = True
        def __init__(self,device):
            self.device = device
            self.UIButton_ID = 11002
        def render_GET(self,request):
            try:
                n = (int)(request.args.get('click')[0])
                d = (int)(request.args.get('device')[0])
                p = (int)(request.args.get('port')[0])
                n = True if n > 0 else False
                print self.device.objects
                for i in range(0,len(self.device.objects)):
                    obj = self.device.objects[i]
                    if obj.getID() == self.UIButton_ID:
                        if obj.getProperty(1) == d and obj.getProperty(2) == p:
                            obj.setProperty(0,n)
                            print "(device, port) = (%d, %d) set property" %  (obj.getProperty(1), obj.getProperty(2))
                        else:
                            print "(device, port) = (%d, %d) doesn't set property" %  (obj.getProperty(1), obj.getProperty(2))
            except:
                pass
            return 'click=%d, device=%d, port=%d' % (n, d, p)

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)
        def init(self):
            cls = UIButton()
            self.addClass(cls,self.FLAG_APP_CAN_CREATE_INSTANCE| self.FLAG_VIRTUAL)

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
    site=server.Site(root)
    site.device = d
    reactor.listenTCP(11002,site)
    reactor.run()

