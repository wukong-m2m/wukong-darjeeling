import traceback
import time, sys 
from udpwkpf import WuClass, Device
from twisted.internet import reactor
import pyupm_rfr359f
from math import log

PIN = 2 #digital pin D2

class IR_sensor(WuClass):
    def __init__(self):
        self.ID = 1010
        self.ir_sensor = pyupm_rfr359f.RFR359F(PIN)
        reactor.callLater(0.5,self.refresh)
        print "temperature sensor init!"

    def update(self,obj,pID,value):
        pass

    def refresh(self):
        self.detected = self.ir_sensor.objectDetected()
        print "WKPFUPDATE(Detected): " + str(self.detected)
        reactor.callLater(0.5,self.refresh)

class MyDevice(Device):
    def __init__(self,addr,localaddr):
        Device.__init__(self,addr,localaddr)

    def init(self):
        m = IR_sensor()
        self.addClass(m,1)
        self.obj_ir = self.addObject(m.ID)
    
    def loop(self):
        print "WKPFUPDATE(Detected): " + str(self.detected)
        self.obj_ir.setProperty(0, self.m.detected)
        reactor.callLater(0.5,self.loop)

if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python udpwkpf.py 127.0.0.1 3000'
        sys.exit(-1)

d = MyDevice(sys.argv[1],sys.argv[2])

reactor.run()

