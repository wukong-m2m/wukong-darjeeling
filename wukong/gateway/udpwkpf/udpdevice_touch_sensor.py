from twisted.internet import reactor
from udpwkpf import WuClass, Device
import sys
from udpwkpf_io_interface import *

Touch_Sensor_Pin = 4

class TouchSensor(WuClass):
    def __init__(self):
        self.ID = 1015
        self.current_value = 0
        self.refreshRate = 0.5
        self.IO = pin_mode(Touch_Sensor_Pin, PIN_TYPE_DIGITAL, PIN_MODE_INPUT)
        reactor.callLater(self.refreshRate, self.refresh)

    def update(self,obj,pID,val):
        if pID == 1:
            self.refreshRate = val/1000.0

    def refresh(self):
        try:
            self.current_value = digital_read(self.IO)
            reactor.callLater(self.refreshRate, self.refresh)
            print "Touchsensor value: %d" % self.current_value
        except IOError:
            print "Error"
            reactor.callLater(self.refreshRate, self.refresh)

if __name__ == "__main__":
    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)
        def init(self):
            m = TouchSensor()
            self.addClass(m,0)
            self.obj_touch_sensor = self.addObject(m.ID)
            reactor.callLater(0.1, self.loop)

        def loop(self):
            self.obj_touch_sensor.setProperty(0, self.obj_touch_sensor.cls.current_value)
            reactor.callLater(0.1, self.loop)

    if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python udpwkpf.py 127.0.0.1 3000'
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()

