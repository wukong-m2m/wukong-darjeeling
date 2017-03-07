from twisted.internet import reactor
from udpwkpf import WuClass, Device
import sys

if __name__ == "__main__":

    class Threshold(WuClass):
        def __init__(self):
            WuClass.__init__(self)
            self.loadClass('Threshold')
            print "Threshold init success"
        def update(self,obj,pID,val):
            if pID == 2:
                op = obj.getProperty(0)
                if op == 0:
                    if val < obj.getProperty(1):
                        obj.setProperty(3,True)
                        print "value %d < threshold %d: True" % (val, obj.getProperty(1))
                    else:
                        obj.setProperty(3,False)
                        print "value %d < threshold %d: False" % (val, obj.getProperty(1))
                elif op == 1:
                    if val > obj.getProperty(1):
                        obj.setProperty(3,True)
                        print "value %d > threshold %d: True" % (val, obj.getProperty(1))
                    else:
                        obj.setProperty(3,False)
                        print "value %d > threshold %d: False" % (val, obj.getProperty(1))
                elif op == 2:
                    if val <= obj.getProperty(1):
                        obj.setProperty(3,True)
                        print "value %d <= threshold %d: True" % (val, obj.getProperty(1))
                    else:
                        obj.setProperty(3,False)
                        print "value %d <= threshold %d: False" % (val, obj.getProperty(1))
                elif op == 3:
                    if val >= obj.getProperty(1):
                        obj.setProperty(3,True)
                        print "value %d >= threshold %d: True" % (val, obj.getProperty(1))
                    else:
                        obj.setProperty(3,False)
                        print "value %d >= threshold %d: False" % (val, obj.getProperty(1))
                else:
                    print "Error: unknown operator %d" % op

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
            cls = Threshold()
            self.addClass(cls, self.FLAG_VIRTUAL)
            self.obj_threshold = self.addObject(cls.ID)

    if len(sys.argv) <= 2:
        print 'python %s <gip> <dip>:<port>' % sys.argv[0]
        print '      <gip>: IP addrees of gateway'
        print '      <dip>: IP address of Python device'
        print '      <port>: An unique port number'
        print ' ex. python %s 192.168.4.7 127.0.0.1:3000' % sys.argv[0]
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
