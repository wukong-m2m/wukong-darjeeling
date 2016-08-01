from twisted.internet import reactor
from udpwkpf import WuClass, Device
import sys
import mraa
import serial
import time

Button_Pin = 5

class Obd2(WuClass):
    def __init__(self):
        WuClass.__init__(self)
        self.loadClass('Obd2')
        self.u = mraa.Uart(0)
        self.p = serial.Serial(self.u.getDevicePath(), baudrate=38400, timeout=0.1)
        self.string = ''
        self.cnt = 0
        self.writeData('ATZ\r', 1.5)
        self.writeData('ATI\r', 0.5)
        self.writeData('ATE0\r', 0.5)
        self.writeData('ATL0\r', 0.5)
        self.writeData('ATSP0\r', 0.5)
        self.writeData('0100\r', 5)

    def writeData(self, string, stime):
        self.p.write(string)
        time.sleep(stime)
        return self.readData()

    def readData(self):
        return self.p.read(100)
    
    def update(self,obj,pID=None,val=None):
        try:
            if self.cnt == 0:
                d = self.writeData('01 10\r', 0.5)
                a = int(d[6:8], 16)
                b = int(d[9:11], 16)
                output = (a * 256 + b) / 100
                print 'Air flow rate:', output
                obj.setProperty(0, output)
            elif self.cnt == 1:
                d = self.writeData('01 05\r', 0.5)
                a = int(d[6:8], 16)
                output = a - 40
                print 'coolant temp:', output
                obj.setProperty(1, output)
            elif self.cnt == 2:
                d = self.writeData('01 0C\r', 0.5)
                a = int(d[6:8], 16)
                b = int(d[9:11], 16)
                output = (a * 256 + b) / 4
                print 'RPM:', output
                obj.setProperty(2, output)
            elif self.cnt == 3:
                d = self.writeData('01 0D\r', 0.5)
                a = int(d[6:8], 16)
                output = a
                print 'speed:', output
                obj.setProperty(3, output)
            elif self.cnt == 4:
                d = self.writeData('01 0F\r', 0.5)
                a = int(d[6:8], 16)
                output = a - 40
                print 'IAT temp:', output
                obj.setProperty(4, output)
            elif self.cnt == 5:
                d = self.writeData('01 11\r', 0.5)
                a = int(d[6:8], 16)
                output = a * 100 / 255
                print 'coolant temp:', output
                obj.setProperty(5, output)
            self.cnt += 1
            self.cnt %= 6
        except IOError:
            print "Error"

if __name__ == "__main__":
    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
            m = Obd2()
            self.addClass(m,0)
            self.obj_obd2 = self.addObject(m.ID)

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
