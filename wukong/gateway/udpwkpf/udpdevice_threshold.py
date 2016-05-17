from udpwkpf import WuClass, Device
import sys

from twisted.protocols import basic
from twisted.internet import reactor, protocol

if __name__ == "__main__":

    class Threshold(WuClass):
        def __init__(self):
            WuClass.__init__(self)
            self.loadClass('Threshold')
            print "Threshold init success"
        def update(self,obj,pID,val):
            print "Threshold input value: ", val
            if pID == 2:
                op = obj.getProperty(0)
                if op == 0:
                    if val < obj.getProperty(1):
                        print "op= ", op, " True"
                        obj.setProperty(3,True)
                    else:
                        print "op= ", op, " False"
                        obj.setProperty(3,False)
                elif op == 1:
                    if val > obj.getProperty(1):
                        print "op= ", op, " True"
                        obj.setProperty(3,True)
                    else:
                        print "op= ", op, " False"
                        obj.setProperty(3,False)
                elif op == 2:
                    if val <= obj.getProperty(1):
                        print "op= ", op, " True"
                        obj.setProperty(3,True)
                    else:
                        print "op= ", op, " False"
                        obj.setProperty(3,False)
                elif op == 3:
                    if val >= obj.getProperty(1):
                        print "op= ", op, " True"
                        obj.setProperty(3,True)
                    else:
                        print "op= ", op, " False"
                        obj.setProperty(3,False)
                else:
                    print "Error: unknown operator %d" % op
            pass
        def init(self):
            pass

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
            cls = Threshold()
            self.addClass(cls, self.FLAG_VIRTUAL)
            self.obj_threshold = self.addObject(cls.ID)

    if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python <filename> <gateway ip> <local ip>:<any given port number>'
        print ' ex. python udpdevice_grove_kit_sample.py 192.168.4.7 127.0.0.1:3000'
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
