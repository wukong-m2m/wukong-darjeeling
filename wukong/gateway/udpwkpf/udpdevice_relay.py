from udpwkpf import WuClass, Device
import sys
from udpwkpf_io_interface import *
from twisted.internet import reactor

Relay_Pin = 8

if __name__ == "__main__":
    class Relay(WuClass):
        def __init__(self):
            WuClass.__init__(self)
            self.loadClass('Relay')
            self.relay_gpio = pin_mode(Relay_Pin, PIN_TYPE_DIGITAL, PIN_MODE_OUTPUT)
            print "Relay init success"

        def update(self,obj,pID=None,val=None):
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
            cls = Relay()
            self.addClass(cls,0)
            self.obj_relay = self.addObject(cls.ID)

    if len(sys.argv) <= 2:
        print 'python %s <gip> <dip>:<port>' % sys.argv[0]
        print '      <gip>: IP addrees of gateway'
        print '      <dip>: IP address of Python device'
        print '      <port>: An unique port number'
        print ' ex. python %s 192.168.4.7 127.0.0.1:3000' % sys.argv[0]
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
