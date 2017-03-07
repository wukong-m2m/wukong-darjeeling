import traceback
import time,sys
from udpwkpf import WuClass, Device
from twisted.internet import reactor
from udpwkpf_io_interface import *

Trig_Pin = 11
Echo_Pin = 12

class Ultrasound_sensor(WuClass):
    def __init__(self):
        WuClass.__init__(self)
        self.loadClass('Ultrasound_Sensor')
        print "Ultrasound sensor init!"

    def update(self,obj,pID=None,val=None):
        trig_gpio = pin_mode(Trig_Pin, PIN_TYPE_DIGITAL, PIN_MODE_OUTPUT)
        digital_write(trig_gpio, 0)
        time.sleep(0.005)
        digital_write(trig_gpio, 1)
        time.sleep(0.002)
        digital_write(trig_gpio, 0)

        echo_gpio = pin_mode(Echo_Pin, PIN_TYPE_DIGITAL, PIN_MODE_INPUT)
        centimeter = int(self.pulseIn(echo_gpio))
        if centimeter >= 0:
            obj.setProperty(0, centimeter)
            print "cm: %d" % centimeter
        else :
            print "no value this time"

    def pulseIn(self, echo_gpio):
        pulseOn = -1
        pulseOff = -1

        count1 = 0
        while digital_read(echo_gpio) == 0 and count1 < 5000:
            pulseOff = time.time()
            count1 += 1
        count2 = 0
        while digital_read(echo_gpio) == 1 and count2 < 5000:
            pulseOn = time.time()
            count2 += 1
        if pulseOn == -1 or pulseOff == -1 or count1 >= 5000 or count2 >= 5000:
            return -1
        timeDifference = pulseOn - pulseOff

        #should convert under fomula for different companies' sensor
        return timeDifference * 17000 #cm
        #return int((pulse_time/2.0)*0.0343);

class MyDevice(Device):
    def __init__(self,addr,localaddr):
        Device.__init__(self,addr,localaddr)

    def init(self):
        m = Ultrasound_sensor()
        self.addClass(m, 0)
        self.obj_ultrasound_sensor = self.addObject(m.ID)

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
