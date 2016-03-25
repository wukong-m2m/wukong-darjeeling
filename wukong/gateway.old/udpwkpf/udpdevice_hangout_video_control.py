from twisted.internet import reactor
from udpwkpf import WuClass, Device
import sys

import HangoutControl; HC = HangoutControl.HangoutControl
import VideoControl; VC = VideoControl.VideoControl

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
            self.ID = 2014
        def update(self,obj,pID,val):
            if pID == 0 or pID == 1:
                print "NUMBER got", val
            else:
                print "NUMBER garbage"

    class HangoutControl(WuClass):
        def __init__(self):
            self.ID = 2034
            self.hc = HC()

        def update(self,obj,pID,val):
            if pID == 0:
                self.hc.update(val, obj.getProperty(1))
            elif pID == 1:
                self.hc.update(obj.getProperty(0), val)

    class VideoControl(WuClass):
        def __init__(self):
            self.ID = 2035
            self.vc = VC()
        def update(self,obj,pID,val):
            if pID == 0:
                self.vc.update(val, obj.getProperty(1), obj.getProperty(2), obj.getProperty(3), obj.getProperty(4), obj.getProperty(5))
            elif pID == 1:
                self.vc.update(obj.getProperty(0), val, obj.getProperty(2), obj.getProperty(3), obj.getProperty(4), obj.getProperty(5))
            elif pID == 2:
                self.vc.update(obj.getProperty(0), obj.getProperty(1), val, obj.getProperty(3), obj.getProperty(4), obj.getProperty(5))
            elif pID == 3:
                self.vc.update(obj.getProperty(0), obj.getProperty(1), obj.getProperty(2), val, obj.getProperty(4), obj.getProperty(5))
            elif pID == 4:
                self.vc.update(obj.getProperty(0), obj.getProperty(1), obj.getProperty(2), obj.getProperty(3), val, obj.getProperty(5))
            elif pID == 5:
                self.vc.update(obj.getProperty(0), obj.getProperty(1), obj.getProperty(2), obj.getProperty(3), obj.getProperty(4), val)

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
            # m = Magnetic()
            # self.addClass(m,1)
            # self.obj_sensor = self.addObject(m.ID)
            # m = Threshold()
            # self.addClass(m,0)
            # self.obj_th = self.addObject(m.ID)
            # m = Slider()
            # self.addClass(m,1)
            # self.obj_slider = self.addObject(m.ID)
            m = Number()
            self.addClass(m,0)
            self.obj_num = self.addObject(m.ID)

            m = VideoControl()
            self.addClass(m,1)
            self.obj_vc = self.addObject(m.ID)

            m = HangoutControl()
            self.addClass(m,1)
            self.obj_hc = self.addObject(m.ID)

    if len(sys.argv) <= 2:
            print 'python udpwkpf.py <ip> <port>'
            print '      <ip>: IP of the interface'
            print '      <port>: The unique port number in the interface'
            print ' ex. python udpwkpf.py 127.0.0.1 3000'
            sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
