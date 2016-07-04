import traceback
import time,sys
from udpwkpf import WuClass, Device
from twisted.internet import reactor
from math import log
from udpwkpf_io_interface import *

Data_Pin = 15
Clk_Pin  = 13

class Temperature_Humidity_Sensor(WuClass):
    def __init__(self):
        WuClass.__init__(self)
        self.loadClass('Temperature_Humidity_Sensor')
        self.temp_humid_obj = sht1x(Data_Pin, Clk_Pin)
        print "temperature humidity sensor init!"

    def update(self,obj,pID=None,val=None):
        try:
            current_temperature = int(sht1x_read_temperature(self.temp_humid_obj)) 
            obj.setProperty(0, current_temperature)
            current_humidity = int(sht1x_read_humidity(self.temp_humid_obj)) 
            obj.setProperty(1, current_humidity)
            print "WKPFUPDATE(Temperature): %d degrees Celsius" % current_temperature
            print "WKPFUPDATE(Humidity): %d percent(%%)" % current_humidity 
        except IOError: 
            print ("Error")

class MyDevice(Device):
    def __init__(self,addr,localaddr):
        Device.__init__(self,addr,localaddr)

    def init(self):
        m = Temperature_Humidity_Sensor()
        self.addClass(m, 0)
        self.obj_temperature_humidity_sensor = self.addObject(m.ID)

if len(sys.argv) <= 2:
        print 'python %s <gip> <dip>:<port>' % sys.argv[0]
        print '      <gip>: IP addrees of gateway'
        print '      <dip>: IP address of Python device'
        print '      <port>: An unique port number'
        print ' ex. python %s 192.168.4.7 127.0.0.1:3000' % sys.argv[0]
        sys.exit(-1)

d = MyDevice(sys.argv[1],sys.argv[2])

reactor.run()
device_cleanup()
