from twisted.internet import reactor
from udpwkpf import WuClass, Device
import sys
from udpwkpf_io_interface import *

Slider_Pin = 0

if __name__ == "__main__":
    class Slider(WuClass):
        def __init__(self):
            WuClass.__init__(self)
            self.loadClass('Slider')
            self.slider_aio = pin_mode(Slider_Pin, PIN_TYPE_ANALOG)
            print "Slider init success"

        def update(self,obj,pID=None,val=None):
            try:
                current_value = analog_read(self.slider_aio)/4 #4 is divisor value which depends on the slider.
                obj.setProperty(2, current_value)
                print "Slider analog pin: ", Slider_Pin, ", value: ", current_value
            except IOError:
                print ("Error")

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
            cls = Slider()
            self.addClass(cls,0)
            self.obj_slider = self.addObject(cls.ID)

    if len(sys.argv) <= 2:
        print 'python %s <gip> <dip>:<port>' % sys.argv[0]
        print '      <gip>: IP addrees of gateway'
        print '      <dip>: IP address of Python device'
        print '      <port>: An unique port number'
        print ' ex. python %s 192.168.4.7 127.0.0.1:3000' % sys.argv[0]
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
