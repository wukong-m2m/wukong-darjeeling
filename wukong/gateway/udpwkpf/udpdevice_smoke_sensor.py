from twisted.internet import reactor
from udpwkpf import WuClass, Device
import sys
from udpwkpf_io_interface import *

Smoke_Pin = 12

class Smoke_Sensor(WuClass):
    def __init__(self):
        WuClass.__init__(self)
        self.loadClass('Smoke_Sensor')
        self.IO = pin_mode(Smoke_Pin, PIN_TYPE_DIGITAL, PIN_MODE_INPUT)

    def update(self,obj,pID=None,val=None):
        try:
            current_value = digital_read(self.IO)
            if current_value == 1:
                obj.setProperty(0, False)
                print "No Alarm"
            elif current_value == 0:
                obj.setProperty(0, True)
                print "Alarm triggered"
        except IOError:
            print "Error"

if __name__ == "__main__":
    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
            m = Smoke_Sensor()
            self.addClass(m,0)
            self.obj_smoke_sensor = self.addObject(m.ID)

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
