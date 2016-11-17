from twisted.internet import reactor
from udpwkpf import WuClass, Device
import sys


class Fire_Agent(WuClass):
    def __init__(self):
        WuClass.__init__(self)
        self.loadClass('Fire_Agent')
        print "Fire Agent init success"
        self.loc = 0
        self.states = [0,0,0,0,0,0]
    def update(self,obj,pID,val):
        self.loc = (int)(obj.getProperty(0))
        newState = (int)(obj.getProperty(1))
        if newState/10 > 0:
            self.states[newState/10] = newState%10
        if self.loc:
            if self.states[self.loc] or self.state[self.loc-1] and self.state[self.loc+1]:
                obj.setProperty(2, 1)
            elif self.states[self.loc+1] or self.state[self.loc+2]:
                obj.setProperty(2, 2)
            elif self.states[self.loc-1] or self.state[self.loc-2]:
                obj.setProperty(2, 3)
            else:
                obj.setProperty(2, 0)

if __name__ == "__main__":
    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
            cls = Fire_Agent()
            self.addClass(cls, self.FLAG_VIRTUAL)
            self.obj_fire_agent = self.addObject(cls.ID)

    if len(sys.argv) <= 2:
        print 'python %s <gip> <dip>:<port>' % sys.argv[0]
        print '      <gip>: IP addrees of gateway'
        print '      <dip>: IP address of Python device'
        print '      <port>: An unique port number'
        print ' ex. python %s 192.168.4.7 127.0.0.1:3000' % sys.argv[0]
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
