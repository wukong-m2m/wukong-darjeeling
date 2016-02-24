import traceback
import time,sys
from udpwkpf import WuClass, Device
from twisted.internet import reactor
import mraa

PIN_1 = 7 #trig
PIN_2 = 8 #echo

class Ultrasound_sensor(WuClass):
    def __init__(self):
        self.ID = 1011
        #trig
        self.trig = mraa.Gpio(PIN_1)
        self.trig.dir(mraa.DIR_OUT)

        self.echo = mraa.Gpio(PIN_2)
        self.echo.dir(mraa.DIR_IN)
        reactor.callLater(0.5,self.refresh)
        print "Ultrasound sensor init!"

    def update(self,obj,pID,value):
        pass

    def refresh(self):
        self.centimeter = self.pulseIn()

        if self.centimeter >= 0:
            print "cm: {0:.3f}".format(self.centimeter)
        reactor.callLater(0.5,self.refresh)

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
        m = Ultrasound_sensor()
        self.addClass(m,1)
        self.obj_ultrasound_sensor = self.addObject(m.ID)
    
    def loop(self):
        self.obj_ultrasound_sensor.setProperty(0,self.m.centimeter)
        print "Distance: " + str(self.m.centimeter)
        reactor.callLater(0.1,self.loop)

if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python udpwkpf.py 127.0.0.1 3000'
        sys.exit(-1)

d = MyDevice(sys.argv[1],sys.argv[2])

reactor.run()
