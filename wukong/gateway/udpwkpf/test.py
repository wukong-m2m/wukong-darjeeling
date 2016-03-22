import traceback
import time,sys
from udpwkpf import WuClass, Device
from twisted.internet import reactor
import random
from math import log

REFRESH_RATE = 0.5 

class Test(WuClass):
    def __init__(self):
        self.ID = 20001
        self.refresh_rate = REFRESH_RATE
        self.array = []
        reactor.callLater(self.refresh_rate,self.refresh)
        print "array test initalize!"

    def update(self,obj,pID,value):
        pass

    def refresh(self):
        self.array = [random.randint(0,100) for _ in xrange(5)]
        print "array: " , self.array
        reactor.callLater(self.refresh_rate,self.refresh)

class MyDevice(Device):
    def __init__(self,addr,localaddr):
        Device.__init__(self,addr,localaddr)

    def init(self):
        m = Test()
        self.addClass(m,1)
        self.obj_test = self.addObject(m.ID)
        reactor.callLater(0.1,self.loop)
    
    def loop(self):
        self.obj_test.setProperty(0, self.obj_test.cls.array)
        reactor.callLater(0.1,self.loop)

if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python udpwkpf.py 127.0.0.1 3000'
        sys.exit(-1)

d = MyDevice(sys.argv[1],sys.argv[2])

reactor.run()

