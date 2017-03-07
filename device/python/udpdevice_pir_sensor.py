from twisted.internet import reactor
from udpwkpf import WuClass, Device
import sys
from udpwkpf_io_interface import *

PIRSensor_Pin = 5

class PIRSensor(WuClass):
    def __init__(self):
        WuClass.__init__(self)
        self.loadClass('PIR_Sensor')
        self.IO = pin_mode(PIRSensor_Pin, PIN_TYPE_DIGITAL, PIN_MODE_INPUT)

    def update(self,obj,pID=None,val=None):
        try:
            current_value = digital_read(self.IO)
            print "PIR sensor value: %d" % current_value
            obj.setProperty(0, current_value)
        except IOError:
            print "Error"

if __name__ == "__main__":
    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
            m = PIRSensor()
            self.addClass(m,0)
            self.obj_pir_sensor = self.addObject(m.ID)

    if len(sys.argv) <= 2:
        print 'python %s <gip> <dip>:<port>' % sys.argv[0]
        print '      <gip>: IP addrees of gateway'
        print '      <dip>: IP address of Python device'
        print '      <port>: An unique port number'
        print ' ex. python %s 192.168.4.7 127.0.0.1:3000' % sys.argv[0]
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()

