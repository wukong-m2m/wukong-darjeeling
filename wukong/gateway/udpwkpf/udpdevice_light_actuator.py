from udpwkpf import WuClass, Device
import sys
from udpwkpf_io_interface import *
from twisted.internet import reactor

Light_Actuator_Pin = 7

if __name__ == "__main__":
    class Light_Actuator(WuClass):
        def __init__(self):
            WuClass.__init__(self)
            self.loadClass('Light_Actuator')
            self.light_actuator_gpio = pin_mode(Light_Actuator_Pin, PIN_TYPE_DIGITAL, PIN_MODE_OUTPUT)

        def update(self,obj,pID=None,val=None):
            try:
                if pID == 0:
                    if val == True:
                        digital_write(self.light_actuator_gpio, 1)
                        print "Light Actuator On"
                    else:
                        digital_write(self.light_actuator_gpio, 0)
                        print "Light Actuator Off"
            except IOError:
                print ("Error")

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
            cls = Light_Actuator()
            self.addClass(cls,0)
            self.obj_light_actuator = self.addObject(cls.ID)

    if len(sys.argv) <= 2:
        print 'python %s <gip> <dip>:<port>' % sys.argv[0]
        print '      <gip>: IP addrees of gateway'
        print '      <dip>: IP address of Python device'
        print '      <port>: An unique port number'
        print ' ex. python %s 192.168.4.7 127.0.0.1:3000' % sys.argv[0]
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
