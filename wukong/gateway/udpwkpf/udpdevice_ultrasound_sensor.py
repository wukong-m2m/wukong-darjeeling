import traceback
import time,sys
from udpwkpf import WuClass, Device
from twisted.internet import reactor
import mraa

PIN_1 = 7 #trig
PIN_2 = 8 #echo
REFRESH_RATE = 0.5 #sec

class Ultrasound_sensor(WuClass):
    def __init__(self,pin1,pin2):
        WuClass.__init__(self)
        self.ID = 1011
        #trig
        self.trig = mraa.Gpio(pin1)
        self.trig.dir(mraa.DIR_OUT)
        #echo
        self.echo = mraa.Gpio(pin2)
        self.echo.dir(mraa.DIR_IN)

        self.refresh_rate = REFRESH_RATE
        self.centimeter = 0
        reactor.callLater(self.refresh_rate,self.refresh)
        print "Ultrasound sensor init!"

    def update(self,obj,pID,value):
        pass

    def refresh(self):
        self.centimeter = int(self.pulseIn())
        if self.centimeter >= 0:
            print "cm: %d" %(self.centimeter)
        else :
            print "no value this time"
        reactor.callLater(self.refresh_rate,self.refresh)

    def pulseIn(self):
        pulseOn = -1
        pulseOff = -1

        self.trig.write(0)
        time.sleep(0.005)
        self.trig.write(1)
        time.sleep(0.002)
        self.trig.write(0)

        count1 = 0
        while self.echo.read() == 0 and count1 < 5000:
            pulseOff = time.time()
            count1 += 1
        count2 = 0
        while self.echo.read() == 1 and count2 < 5000:
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
        m = Ultrasound_sensor(PIN_1,PIN_2)
        self.addClass(m,1)
        self.obj_ultrasound_sensor = self.addObject(m.ID)
        reactor.callLater(0.1,self.loop)

    def loop(self):
        self.obj_ultrasound_sensor.setProperty(0,self.obj_ultrasound_sensor.cls.centimeter)
        #print "Distance: " + str(self.obj_ultrasound_sensor.cls.centimeter)
        reactor.callLater(0.1,self.loop)

if len(sys.argv) <= 2:
        print 'python %s <gip> <dip>:<port>' % sys.argv[0]
        print '      <gip>: IP addrees of gateway'
        print '      <dip>: IP address of Python device'
        print '      <port>: An unique port number'
        print ' ex. python %s 192.168.4.7 127.0.0.1:3000' % sys.argv[0]
        sys.exit(-1)

d = MyDevice(sys.argv[1],sys.argv[2])

reactor.run()
