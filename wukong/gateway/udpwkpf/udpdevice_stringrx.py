import traceback
import time,sys
from udpwkpf import WuClass, Device
from twisted.internet import reactor
import random
from math import log

class StringRx(WuClass):
    def __init__(self):
        WuClass.__init__(self)
        self.loadClass('StringRx')
        print "StringRx init success"

    def update(self,obj,pID,value):
        val = obj.getProperty(0)
        print "get string: ", val
        if type(val) == str:
            print "string length: ", len(val)

class MyDevice(Device):
    def __init__(self,addr,localaddr):
        Device.__init__(self,addr,localaddr)

    def init(self):
        m = StringRx()
        self.addClass(m,1)
        self.obj_test = self.addObject(m.ID)
    
if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <ip:port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python udpwkpf.py 127.0.0.1 3000'
        sys.exit(-1)

d = MyDevice(sys.argv[1],sys.argv[2])

reactor.run()

