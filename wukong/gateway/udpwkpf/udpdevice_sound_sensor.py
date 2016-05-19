from twisted.internet import reactor
from udpwkpf import WuClass, Device
import sys
from udpwkpf_io_interface import *

Sound_Pin = 2 # depends on which analog port

class Sound_sensor(WuClass):
    def __init__(self):
        WuClass.__init__(self)
        self.loadClass('Sound_Sensor')
        self.sound_sensor_aio = pin_mode(Sound_Pin, PIN_TYPE_ANALOG)
        print "Sound sensor init!"

        def update(self,obj,pID=None,val=None):
            try:
                current_value = analog_read(self.sound_sensor_aio)
                current_value = int(((self.current_value/4095.0)*255)); #should convert under fomula for differnet companies' sensor
                obj.setProperty(0, current_value)
                print "Sound sensor analog pin: %d, value: %d" % (Sound_Pin, current_value)
            except IOError:
                print ("Error")

class MyDevice(Device):
    def __init__(self,addr,localaddr):
        Device.__init__(self,addr,localaddr)

    def init(self):
        self.m = Sound_sensor()
        self.addClass(self.m, 0)
        self.obj_sound_sensor = self.addObject(self.m.ID)

if len(sys.argv) <= 2:
        print 'python %s <gip> <dip>:<port>' % sys.argv[0]
        print '      <gip>: IP addrees of gateway'
        print '      <dip>: IP address of Python device'
        print '      <port>: An unique port number'
        print ' ex. python %s 192.168.4.7 127.0.0.1:3000' % sys.argv[0]
        sys.exit(-1)

d = MyDevice(sys.argv[1],sys.argv[2])

reactor.run()

