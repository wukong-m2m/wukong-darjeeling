from udpwkpf import WuClass, Device
import sys
from twisted.protocols import basic
from twisted.internet import reactor, protocol
from twisted.web import resource,static,server
import traceback

global site

if __name__ == "__main__":
    class UISlider(WuClass):
        def __init__(self):
            self.loadClass('UISlider')
        def udpate(self,obj,pID,val):
            pass
        def init(self):
            pass

    class Slide(resource.Resource):
        isLeaf = True
        def __init__(self,device):
            self.device = device
            self.UISlider_ID = 11006

        def render_GET(self,request):
            try:
                n = (int)(request.args.get('number')[0])
                d = (int)(request.args.get('device')[0])
                p = (int)(request.args.get('port')[0])
                print self.device.objects
                for i in range(0,len(self.device.objects)):
                    obj = self.device.objects[i]
                    if obj.getID() == self.UISlider_ID:
                        if obj.getProperty(1) == d and obj.getProperty(2) == p:
                            obj.setProperty(0,n)
                            print "(device, port) = (%d, %d) set property" %  (obj.getProperty(1), obj.getProperty(2))
                        else:    
                            print "(device, port) = (%d, %d) doesn't set property" %  (obj.getProperty(1), obj.getProperty(2))
            except:
                pass
            return 'number=%d, device=%d, port=%d' % (n, d, p)

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)
        def init(self):
            cls = UISlider()
            self.addClass(cls, self.FLAG_APP_CAN_CREATE_INSTANCE| self.FLAG_VIRTUAL)

    if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python <filename> <gateway ip> <local ip>:<any given port number>'
        print ' ex. python udpdevice_eeg_server.py 192.168.4.7 127.0.0.1:3000'
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    root=static.File("./www")
    root.putChild("slider",Slide(d))
    site=server.Site(root)
    site.device = d
    reactor.listenTCP(11006,site)
    reactor.run()

