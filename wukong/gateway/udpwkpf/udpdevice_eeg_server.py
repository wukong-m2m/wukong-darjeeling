from udpwkpf import WuClass, Device
import sys
import mraa
from twisted.protocols import basic
from twisted.internet import reactor, protocol

PORT = 2222

if __name__ == "__main__":
    class Number(WuClass):  #"Number" WuClass has been defined in ~/wukong-darjeeling/wukong/ComponentDefinitions/WuKongStandardLibrary.xml
        def __init__(self):
            WuClass.__init__(self)
            self.ID = 2014  #2014 is WuClass id of "Number"
            print "Number init success"
        def update(self,obj,pID,val):
            if pID == 0 or pID == 1: # pID is property ID, 0 is the first property of "Number"
                print "NUMBER(int) is %d" % val
            else:
                print "NUMBER(boolean) is ", val

    class Light_Actuator(WuClass):
        def __init__(self, pin):
            WuClass.__init__(self)
            self.ID = 2001
            self.light_actuator_gpio = mraa.Gpio(pin)
            self.light_actuator_gpio.dir(mraa.DIR_OUT)
            print "Light Actuator init success"

        def update(self,obj,pID,val):
            if pID == 0:
                if val == True:
                    self.light_actuator_gpio.write(1)
                    print "Light Actuator On"
                else:
                    self.light_actuator_gpio.write(0)
                    print "Light Actuator Off"
            else:
                print "Light Actuator garbage"

    class EEGServerProtocol(basic.LineReceiver):
        def connectionMade(self):
            print "Got new client!"
            self.factory.clients.append(self)

        def connectionLost(self, reason):
            print "Lost a client!", str(reason)
            self.factory.clients.remove(self)

        def dataReceived(self, data):
            data = data[data.find("#"):data.find("@")]
            if len(data) < 3: return

            print "received", repr(data), map(ord, data)
            output_short = int(data[1])
            output_boolean = bool(int(data[2]))
            print "output_short: ", output_short, " output_boolean: ", output_boolean
            self.factory.obj_eeg_server.setProperty(0, output_short) # setProperty has two parameters, the first is pID, the second is output value
            self.factory.obj_eeg_server.setProperty(1, output_boolean)

    class EEGServerFactory(protocol.ServerFactory):
        protocol = EEGServerProtocol
        def __init__(self, obj):
            self.clients = []
            self.obj_eeg_server = obj

    class EEGServer(WuClass):
        def __init__(self):
            WuClass.__init__(self)
            self.ID = 1912
            print "EEGServer init success"

        def update(self,obj,pID,val):
            pass

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):

            m1 = Number()
            self.addClass(m1,1)
            self.obj_num = self.addObject(m1.ID)

            Light_Actuator_Pin = 2
            m2 = Light_Actuator(Light_Actuator_Pin)
            self.addClass(m2,0)
            self.obj_light_actuator = self.addObject(m2.ID)

            m3 = EEGServer()
            self.addClass(m3,1)
            self.obj_eeg_server = self.addObject(m3.ID)

    if len(sys.argv) <= 2:
        print 'python %s <gip> <dip>:<port>' % sys.argv[0]
        print '      <gip>: IP addrees of gateway'
        print '      <dip>: IP address of Python device'
        print '      <port>: An unique port number'
        print ' ex. python %s 192.168.4.7 127.0.0.1:3000' % sys.argv[0]
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    factory = EEGServerFactory(d.obj_eeg_server)
    reactor.listenTCP(PORT, factory)

    reactor.run()
