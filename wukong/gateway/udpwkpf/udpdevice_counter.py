from twisted.internet import reactor
from udpwkpf import WuClass, Device
import sys, time
from udpwkpf_io_interface import *

Counter_Pin = 7
HIGH = 1
LOW  = 0

class Counter(WuClass):
    def __init__(self):
        WuClass.__init__(self)
        self.loadClass('Counter')
        self.IO = pin_mode(Counter_Pin, PIN_TYPE_DIGITAL, PIN_MODE_INPUT)
        self.count = 0
        self.previous = LOW
        self.time = 0
        self.debounce = 100

    def update(self,obj,pID=None,val=None):
        try:
            current_value = digital_read(self.IO)
            currentTime = int(time.time() * 100)
            if (current_value == HIGH and self.previous == LOW and currentTime - self.time > self.debounce):
              self.count = (self.count + 1) % 5
              obj.setProperty(0, self.count)
              print "Output index: %d" % self.count
              self.time = currentTime
            else:
              pass
            self.previous = current_value
        except IOError:
            print "Error"

if __name__ == "__main__":
    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
            m = Counter()
            self.addClass(m,0)
            self.obj_counter = self.addObject(m.ID)

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
