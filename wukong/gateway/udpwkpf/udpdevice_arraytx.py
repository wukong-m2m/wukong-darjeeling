import traceback
import time,sys
from udpwkpf import WuClass, Device
from twisted.internet import reactor
import random
from math import log

class ArrayTx(WuClass):
    def __init__(self):
        WuClass.__init__(self)
        self.loadClass('ArrayTx')
        print "ArrayTx init success"

    def update(self,obj,pID,value):
        self.array = [1,2,3]
        obj.setProperty(0, self.array)
        print "ArrayTx: " , self.array

class MyDevice(Device):
    def __init__(self,addr,localaddr):
        Device.__init__(self,addr,localaddr)

    def init(self):
        m = ArrayTx()
        self.addClass(m,1)
        self.obj_test = self.addObject(m.ID)

if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python udpwkpf.py 127.0.0.1 3000'
        sys.exit(-1)

d = MyDevice(sys.argv[1],sys.argv[2])

reactor.run()

