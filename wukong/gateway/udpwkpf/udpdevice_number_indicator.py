from udpwkpf import WuClass, Device
import sys
from twisted.protocols import basic
from twisted.internet import reactor, protocol
from twisted.web import resource,static,server
import traceback

if __name__ == "__main__":
    class Number(WuClass):  
        def __init__(self):
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
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python <filename> <gateway ip> <local ip>:<any given port number>'
        print ' ex. python udpdevice_eeg_server.py 192.168.4.7 127.0.0.1:3000'
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()

