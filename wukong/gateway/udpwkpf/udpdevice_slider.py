from udpwkpf import WuClass, Device
import sys
import mraa

from twisted.protocols import basic
from twisted.internet import reactor, protocol

if __name__ == "__main__":
    class Slider(WuClass):
        def __init__(self):
            self.ID = 1006
        
        def setup(self, obj, pin):
            slider_aio = mraa.Aio(pin)
            print "Slider init success"
            reactor.callLater(0.1, self.refresh, obj, pin, slider_aio)

        def refresh(self, obj, pin, slider_aio):
            current_value = slider_aio.read() / 4
            obj.setProperty(2, current_value)
            print "Slider pin: ", pin, ", value: ", current_value
            reactor.callLater(0.1, self.refresh, obj, pin, slider_aio)
            
    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):

            cls = Slider()
            self.addClass(cls,0)
            self.obj_slider = self.addObject(cls.ID)
            self.obj_slider.cls.setup(self.obj_slider, 0)

    if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python <filename> <gateway ip> <local ip>:<any given port number>'
        print ' ex. python udpdevice_grove_kit_sample.py 192.168.4.7 127.0.0.1:3000'
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
