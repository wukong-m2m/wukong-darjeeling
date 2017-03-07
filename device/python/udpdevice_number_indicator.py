from udpwkpf import WuClass, Device
import sys
from twisted.protocols import basic
from twisted.internet import reactor, protocol
from twisted.web import resource,static,server
import traceback

if __name__ == "__main__":
    class Number(WuClass):
        def __init__(self):
            WuClass.__init__(self)
            self.loadClass('Number')
            print "Number init success"
        def update(self,obj,pID,val):
            if pID == 0:
                print "NUMBER(int) is %d" % val
            elif pID == 1:
                print "NUMBER(boolean) is %d" % val
            else:
                print "Number garbage"

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)
        def init(self):
            cls = Number()
            self.addClass(cls, self.FLAG_VIRTUAL)
            self.obj_number_indicator = self.addObject(cls.ID)

    if len(sys.argv) <= 2:
        print 'python %s <gip> <dip>:<port>' % sys.argv[0]
        print '      <gip>: IP addrees of gateway'
        print '      <dip>: IP address of Python device'
        print '      <port>: An unique port number'
        print ' ex. python %s 192.168.4.7 127.0.0.1:3000' % sys.argv[0]
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()

