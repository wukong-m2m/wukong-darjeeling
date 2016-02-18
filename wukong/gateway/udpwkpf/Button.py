from twisted.internet import reactor
from udpwkpf import WuClass, Device
import sys
import mraa

class Button(WuClass):
    def __init__(self):
        self.ID = 1012
        self.current_value = 0
        self.refreshRate = 0.1
        self.IO = mraa.Gpio(5)
        self.IO.dir(mraa.DIR_IN)
        reactor.callLater(self.refreshRate, self.refresh)

    def update(self,obj,pID,val):
        if pID == 1:
            self.refreshRate = val/1000.0

    def refresh(self):
        self.current_value = self.IO.read()
        reactor.callLater(self.refreshRate, self.refresh)


if __name__ == "__main__":
    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)
        def init(self):
            m = Button()
            self.addClass(m,0)
            self.obj_button = self.addObject(m.ID)
            reactor.callLater(0.1, self.loop)

        def loop(self):
            self.obj_button.setProperty(0, self.m.current_value)
            reactor.callLater(0.1, self.loop)

    if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python udpwkpf.py 127.0.0.1 3000'
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()

