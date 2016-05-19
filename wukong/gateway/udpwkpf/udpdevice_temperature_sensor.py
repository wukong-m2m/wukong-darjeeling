import traceback
import time,sys
from udpwkpf import WuClass, Device
from twisted.internet import reactor
from math import log
from udpwkpf_io_interface import *

PIN = 3 #Analog pin 0

class Temperature_sensor(WuClass):
    def __init__(self):
        WuClass.__init__(self)
        self.loadClass('Temperature_Sensor')
        print "temperature sensor init!"

    def update(self,obj,pID=None,val=None):
        try:
            current_value = temp_read(PIN)
            obj.setProperty(0, current_value)
            print "WKPFUPDATE(Temperature): %d degrees Celsius" % current_value
        except IOError:
            print ("Error")

class MyDevice(Device):
    def __init__(self,addr,localaddr):
        Device.__init__(self,addr,localaddr)

    def init(self):
        m = Temperature_sensor()
        self.addClass(m, 0)
        self.obj_temperature_sensor = self.addObject(m.ID)

if len(sys.argv) <= 2:
        print 'python %s <gip> <dip>:<port>' % sys.argv[0]
        print '      <gip>: IP addrees of gateway'
        print '      <dip>: IP address of Python device'
        print '      <port>: An unique port number'
        print ' ex. python %s 192.168.4.7 127.0.0.1:3000' % sys.argv[0]
        sys.exit(-1)

d = MyDevice(sys.argv[1],sys.argv[2])

reactor.run()

