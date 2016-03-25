from twisted.web.client import FileBodyProducer
from twisted.protocols import basic
from twisted.internet import reactor
from StringIO import StringIO

from udpwkpf import WuClass, Device
import sys
import mraa

import socket, struct
import time
import philip_hue_utils; HWC = philip_hue_utils.hue_web_client 
import philip_hue_utils; HC = philip_hue_utils.hue_calculation 

if __name__ == "__main__":
    class Gamma():
        def __init__(self):
            self.gamma = -1
            self.x = -1    
            self.y = -1    
            self.bri = -1
            self.message = ""

    class Philip_Hue_Go_Actuator(WuClass):
        def __init__(self):
            self.ID = 2017
            self.lasttime = int(round(time.time() * 1000))
            self.loop_rate = 500
            self.gamma = Gamma()
            print "Hue Go Actuator init success"

        def update(self,obj,pID,val):
            debug_name = "Philip_Hue_Go_Actuator"
            user = "newdeveloper"
            ip = ((int)(obj.getProperty(0)) & 0xffff) << 16
            ip |= ((int)(obj.getProperty(1)) & 0xffff)
            ip = socket.inet_ntoa(struct.pack('!L',ip))
            index = obj.getProperty(2)
            on = obj.getProperty(3)
            r = obj.getProperty(4)
            g = obj.getProperty(5)
            b = obj.getProperty(6)
            hwc = HWC(ip, user, index, self.gamma)

            currenttime = int(round(time.time() * 1000))
            
            if (currenttime - self.lasttime > self.loop_rate):
                if(self.gamma.gamma < 0):
                    hwc.get_gamma()
                    if (self.gamma.gamma < 0):
                        print "\n_____%s_____GET gamma error:%d\n" % (debug_name, self.gamma.gamma)
                        if (self.gamma.gamma < -99):
                            print "\n_____%s_____JSON error:%s\n" % (debug_name, self.gamma.message)
                        else:
                            print "\n_____%s____Error!ip:%s,index:%d\n" % (debug_name, ip, index)
                    self.lasttime = currenttime
                    return

                hc = HC()
                hc.RGBtoXY(self.gamma, r, g, b)

                if(on):
                    command = ("{\"on\":true, \"xy\":[%.2f,%.2f], \"bri\":%d}" % (self.gamma.x, self.gamma.y, (int)(self.gamma.bri*255)))
                else:
                    command = ("{\"on\":false}")
            
                time.sleep(0.05)
                print "\n_____%s_____PUT command:%s\n" % (debug_name, command)

                body = FileBodyProducer(StringIO(command))
                hwc.put_command(body)
                self.lasttime = currenttime

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
            self.m = Philip_Hue_Go_Actuator()
            self.addClass(self.m,0)
            self.obj_philip = self.addObject(self.m.ID)

    if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python udpdevice*.py <gateway IP> <locolhost>:<port>'
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
