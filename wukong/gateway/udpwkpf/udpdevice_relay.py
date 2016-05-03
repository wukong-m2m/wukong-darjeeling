from udpwkpf import WuClass, Device
import sys
from udpwkpf_io_interface import *
from twisted.internet import reactor

Relay_Pin = 8

if __name__ == "__main__":
    class Relay(WuClass):
        def __init__(self, pin):
            self.ID = 2009
            self.relay_gpio = pin_mode(pin, PIN_TYPE_DIGITAL, PIN_MODE_OUTPUT)
            print "Relay init success"

        def update(self,obj,pID,val):
            try:
                if pID == 0:
                    if val == True:
                        digital_write(self.relay_gpio, 1)
                        print "Relay On"
                    else:
                        digital_write(self.relay_gpio, 0)
                        print "Relay Off"
                else:
                    print "Relay garbage"
             except IOError:
                print ("Error")

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
            cls = Relay(Relay_Pin)
            self.addClass(cls,0)
            self.obj_relay = self.addObject(cls.ID)

    if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python <filename> <gateway ip> <local ip>:<any given port number>'
        print ' ex. python udpdevice_grove_kit_sample.py 192.168.4.7 127.0.0.1:3000'
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
