from udpwkpf import WuClass, Device
import sys
import mraa

from twisted.protocols import basic
from twisted.internet import reactor, protocol

if __name__ == "__main__":
    class Light_Actuator(WuClass):
        def __init__(self, pin):
            self.ID = 2001
            self.light_actuator_gpio = mraa.Gpio(pin)
            self.light_actuator_gpio.dir(mraa.DIR_OUT)
            print "Light Actuator init success"

        def update(self,obj,pID,val):
            if pID == 0:
                if val == True:
                    self.light_actuator_gpio.write(1)
                    print "Light Actuator On"
                else:
                    self.light_actuator_gpio.write(0)
                    print "Light Actuator Off"
            else:
                print "Light Actuator garbage"

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
            Light_Actuator_Pin = 2 
            cls = Light_Actuator(Light_Actuator_Pin)
            self.addClass(cls,0)
            self.obj_light_actuator = self.addObject(cls.ID)

    if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python <filename> <gateway ip> <local ip>:<any given port number>'
        print ' ex. python udpdevice_grove_kit_sample.py 192.168.4.7 127.0.0.1:3000'
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
