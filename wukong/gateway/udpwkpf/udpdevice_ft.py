from twisted.internet import reactor
from udpwkpf import WuClass, Device
import sys
import time

FIX_TIME = 3
HEART_BEAT_PERIOD = 0.5

class FT(WuClass):
    def __init__(self):
        WuClass.__init__(self)
        self.loadClass('FT')
        self.enable = False
        self.lastGetHeartBeatTime = time.time()
        self.lastHeartBeatTime = 0
        self.heartBeatCounter = 0

    def update(self,obj,pID=None,val=None):
        if time.time() - self.lastHeartBeatTime > HEART_BEAT_PERIOD:
            self.heartBeatCounter += 1
            if self.heartBeatCounter > 100:
                self.heartBeatCounter %= 100
            obj.setProperty(2, self.heartBeatCounter)
        
        if pID == 0 or pID == 1:
            self.lastGetHeartBeatTime = time.time()
        
        if time.time() - self.lastGetHeartBeatTime > FIX_TIME:
            obj.setProperty(3, True)
        else:
            obj.setProperty(3, False)

if __name__ == "__main__":
    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)
        def init(self):
            cls = FT()
            self.addClass(cls, self.FLAG_APP_CAN_CREATE_INSTANCE| self.FLAG_VIRTUAL)

    if len(sys.argv) <= 2:
        print 'python %s <gip> <dip>:<port>' % sys.argv[0]
        print '      <gip>: IP addrees of gateway'
        print '      <dip>: IP address of Python device'
        print '      <port>: An unique port number'
        print ' ex. python %s 192.168.4.7 127.0.0.1:3000' % sys.argv[0]
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()

