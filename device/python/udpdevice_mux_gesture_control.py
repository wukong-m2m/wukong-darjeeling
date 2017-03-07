from twisted.internet import reactor
from udpwkpf import WuClass, Device
import sys

from twisted.protocols import basic
from twisted.internet import reactor, protocol

import GestureSwitch; MUX = GestureSwitch.GestureSwitch
import GestureToBrightnessControl; G2BC = GestureToBrightnessControl.GestureToBrightnessControl
import GestureToColorControl; G2CC = GestureToColorControl.GestureToColorControl
import GestureToHangoutControl; G2HC= GestureToHangoutControl.GestureToHangoutControl
import GestureToVideoControl; G2VC = GestureToVideoControl.GestureToVideoControl

PORT = 2222


if __name__ == "__main__":
    # class Magnetic(WuClass):
    #     def __init__(self):
    #         self.ID = 1007
    #     def update(self,obj,pID,val):
    #         pass
    # class Threshold(WuClass):
    #     def __init__(self):
    #         self.ID = 1
    #     def update(self,obj,pID,val):
    #         pass
    # class Slider(WuClass):
    #     def __init__(self):
    #         self.ID = 1006
    #     def update(self,obj,pID,val):
    #     #def update(self):
    #         pass
    class Number(WuClass):
        def __init__(self):
            WuClass.__init__(self)
            self.loadClass('Number')
        def update(self,obj,pID,val):
            if pID == 0 or pID == 1:
                print "NUMBER got", val
            else:
                print "NUMBER garbage"

    class RealSenseProtocol(basic.LineReceiver):
        def connectionMade(self):
            print "Got new client!"
            self.factory.clients.append(self)

        def connectionLost(self, reason):
            print "Lost a client!", str(reason)
            self.factory.clients.remove(self)

        def lineReceived(self, data):
            data = data[data.find("#"):data.find("@")]
            if len(data) < 3: return

            print "received", repr(data), map(ord, data)
            mode = int(data[1])
            gesture = int(data[2])

            self.factory.obj_realsense.setProperty(0, False)
            self.factory.obj_realsense.setProperty(1, 7)

            if self.factory.obj_realsense.cls.prev_mode != mode:
                self.factory.obj_realsense.cls.prev_mode = mode
                self.factory.obj_realsense.setProperty(0, True)

            if 0 <= gesture <= 7:
                print 'received gesture', gesture
                self.factory.obj_realsense.setProperty(1, gesture)


    class RealSenseFactory(protocol.ServerFactory):
        protocol = RealSenseProtocol
        def __init__(self, obj):
            self.clients = []
            self.obj_realsense = obj

    class RealSense(WuClass):
        def __init__(self):
            WuClass.__init__(self)
            self.loadClass('RealSense')
            self.prev_mode = -1

        def update(self,obj,pID,val):
            pass

    class GestureSwitch(WuClass):
        def __init__(self):
            WuClass.__init__(self)
            self.loadClass('GestureSwitch')
            self.mux = MUX()
        def update(self,obj,pID,val):
            print "GS properties"
            print obj.getProperty(0)
            print obj.getProperty(1)
            print obj.getProperty(2)
            print "end!!!!!!!!!!!!!!!!!!"
            ret = None
            if pID == 0:
                ret = self.mux.update_bool(val, obj.getProperty(1))
            elif pID == 1:
                ret = self.mux.update_bool(obj.getProperty(0), val)

            if ret is not None:
                print "GS ret =", ret
                map(lambda x: obj.setProperty(x+2, ret[x]), xrange(5))

    class GestureToBrightnessControl(WuClass):
        def __init__(self):
            WuClass.__init__(self)
            self.loadClass('GestureToBrightnessControl')
            self.g2bc = G2BC()
        def update(self,obj,pID,val):
            ret = None
            if pID == 0:
                ret = self.g2bc.update(val)

            if ret is not None:
                map(lambda x: obj.setProperty(x+1, ret[x]), xrange(1))

    class GestureToColorControl(WuClass):
        def __init__(self):
            WuClass.__init__(self)
            self.loadClass('GestureToColorControl')
            self.g2cc = G2CC()
        def update(self,obj,pID,val):
            ret = None
            if pID == 0:
                ret = self.g2cc.update(val)

            if ret is not None:
                map(lambda x: obj.setProperty(x+1, ret[x]), xrange(3))

    class GestureToHangoutControl(WuClass):
        def __init__(self):
            WuClass.__init__(self)
            self.loadClass('GestureToHangoutControl')
        def update(self,obj,pID,val):
            ret = None
            if pID == 0:
                ret = self.g2hc.update(val)

            if ret is not None:
                map(lambda x: obj.setProperty(x+1, ret[x]), xrange(2))

    class GestureToVideoControl(WuClass):
        def __init__(self):
            WuClass.__init__(self)
            self.loadClass('GestureToVideoControl')
            self.g2vc = G2VC()
        def update(self,obj,pID,val):
            print "GGGGGGG", pID, val
            ret = None
            if pID == 0:
                ret = self.g2vc.update(val)

            if ret is not None:
                print "G2VC p",ret[0],"s",ret[1],"n",ret[2],"p",ret[3],"i",ret[4],"d",ret[5]
                map(lambda x: obj.setProperty(x+1, ret[x]), xrange(6))

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):

            m = Number()
            self.addClass(m,1)
            self.obj_num = self.addObject(m.ID)

            m = GestureSwitch()
            self.addClass(m,1)
            self.obj_mux = self.addObject(m.ID)

            # m = GestureToBrightnessControl()
            # self.addClass(m,1)
            # self.obj_g2bc = self.addObject(m.ID)

            # m = GestureToBrightnessControl()
            # self.addClass(m,1)
            # self.obj_g2bc2 = self.addObject(m.ID)

            # m = GestureToColorControl()
            # self.addClass(m,1)
            # self.obj_g2cc = self.addObject(m.ID)

            m = GestureToHangoutControl()
            self.addClass(m,1)
            self.obj_g2hc = self.addObject(m.ID)

            m = GestureToVideoControl()
            self.addClass(m,1)
            self.obj_g2vc = self.addObject(m.ID)

            m = RealSense()
            self.addClass(m,1)
            self.obj_realsense = self.addObject(m.ID)

    if len(sys.argv) <= 2:
        print 'python %s <gip> <dip>:<port>' % sys.argv[0]
        print '      <gip>: IP addrees of gateway'
        print '      <dip>: IP address of Python device'
        print '      <port>: An unique port number'
        print ' ex. python %s 192.168.4.7 127.0.0.1:3000' % sys.argv[0]
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    factory = RealSenseFactory(d.obj_realsense)
    reactor.listenTCP(PORT, factory)

    reactor.run()