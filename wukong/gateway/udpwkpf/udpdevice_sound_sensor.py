import traceback
import time,sys
from udpwkpf import WuClass, Device
from twisted.internet import reactor
from udpwkpf_io_interface import *

PIN = 2 # depends on which analog port
REFRESH_RATE = 0.5

class Sound_sensor(WuClass):
    def __init__(self,pin):
        self.ID = 1014
        self.refresh_rate = REFRESH_RATE
        self.current_value = 0 
        self.sound_sensor_aio = pin_mode(pin, PIN_TYPE_ANALOG)
        reactor.callLater(self.refresh_rate,self.refresh)
        print "Sound sensor init!"

    def update(self,obj,pID,value):
        pass

    def refresh(self):
        try:
            self.current_value = analog_read(self.sound_sensor_aio)
            #should convert under fomula for differnet companies' sensor
            self.current_value = int(((self.current_value/4095.0)*255));
            reactor.callLater(self.refresh_rate,self.refresh)
        except IOError:
            print ("Error")
            reactor.callLater(self.refresh_rate,self.refresh)

class MyDevice(Device):
    def __init__(self,addr,localaddr):
        Device.__init__(self,addr,localaddr)

    def init(self):
        self.m = Sound_sensor(PIN)
        self.addClass(self.m,1)
        self.obj_sound_sensor = self.addObject(self.m.ID)
        reactor.callLater(0.5,self.loop)
    
    def loop(self):
        self.obj_sound_sensor.setProperty(0,self.obj_sound_sensor.cls.current_value)
        print "Sound sensor analog pin: %d, value: %d" % (PIN, self.m.current_value)
        reactor.callLater(0.5,self.loop)


if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python udpwkpf.py 127.0.0.1 3000'
        sys.exit(-1)

d = MyDevice(sys.argv[1],sys.argv[2])

reactor.run()
