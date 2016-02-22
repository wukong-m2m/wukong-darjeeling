import traceback
import time,sys
from udpwkpf import WuClass, Device
from twisted.internet import reactor
import mraa
from math import log

PIN = 2

class Temperature_sensor(WuClass):
    def __init__(self):
        self.ID = 1013
        self.temperature_sensor_aio = mraa.Aio(PIN)
        reactor.callLater(0.5,self.refresh)
        print "temperature sensor init!"

    def update(self,obj,pID,value):
        pass

    def refresh(self):
        output = self.temperature_sensor_aio.read()
        print "raw value: " + str(output)

        #should convert under fomula for different companies' sensor
        resistance = (4095 - output) * 10000 / output;
        self.temperature = 1/(((log(resistance/10000.0))/3975)+(1/298.15))-273.15

        reactor.callLater(0.5,self.refresh)

class MyDevice(Device):
    def __init__(self,addr,localaddr):
        Device.__init__(self,addr,localaddr)

    def init(self):
        m = Temperature_sensor()
        self.addClass(m,1)
        self.obj_temperature_sensor = self.addObject(m.ID)
    
    def loop(self):
        self.obj_temperature_sensor.setProperty(0,self.m.temperature)
        print "WKPFUPDATE(Temperature): " + str(self.m.temperature)
        reactor.callLater(0.1,self.loop)

if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python udpwkpf.py 127.0.0.1 3000'
        sys.exit(-1)

d = MyDevice(sys.argv[1],sys.argv[2])

reactor.run()

