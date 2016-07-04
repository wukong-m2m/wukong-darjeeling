from twisted.internet import reactor
from udpwkpf import WuClass, Device
import sys, time
from twisted.protocols import basic
from twisted.internet import reactor, protocol

from udpwkpf_io_interface import *

HIGH = 1
LOW  = 0

def HSV_2_RGB(HSV):
    # copy from http://web.mit.edu/storborg/Public/hsvtorgb.c
    ''' Converts an integer HSV tuple (value range from 0 to 255) to an RGB tuple '''
    # Unpack the HSV tuple for readability
    H, S, V = HSV
    # Check if the color is Grayscale
    if S == 0:
        R = V
        G = V
        B = V
        return (R, G, B)
    # Make hue 0-5
    region = H // 43;
    # Find remainder part, make it from 0-255
    remainder = (H - (region * 43)) * 6;
    # Calculate temp vars, doing integer multiplication
    P = (V * (255 - S)) >> 8;
    Q = (V * (255 - ((S * remainder) >> 8))) >> 8;
    T = (V * (255 - ((S * (255 - remainder)) >> 8))) >> 8;
    # Assign temp vars based on color cone region
    if region == 0:
        R = V
        G = T
        B = P
    elif region == 1:
        R = Q;
        G = V;
        B = P;
    elif region == 2:
        R = P;
        G = V;
        B = T;
    elif region == 3:
        R = P;
        G = Q;
        B = V;
    elif region == 4:
        R = T;
        G = P;
        B = V;
    else:
        R = V;
        G = P;
        B = Q;

    return (R, G, B)

if __name__ == "__main__":

    class Grove_LCD(WuClass):
        def __init__(self):
            WuClass.__init__(self)
            self.loadClass('Grove_LCD')
            self.myLcd = grove_lcd(0x3E, 0x62)
            print "LCD Actuator init success"
            self.state = LOW
            self.previous = LOW
            self.time = 0
            self.debounce = 200
            self.cleanup = True

        def update(self,obj,pID,val):
            on_off = obj.getProperty(0)
            currentTime = int(time.time() * 100)
            if (on_off == HIGH and self.previous == LOW and currentTime - self.time > self.debounce):
                if (self.state == HIGH):
                    self.state = LOW
                    self.cleanup = False
                else:
                    self.state = HIGH
                self.time = currentTime
            if self.state:
                display_value = obj.getProperty(1)
                bri = obj.getProperty(2)
                hue = obj.getProperty(3)
                saturation = obj.getProperty(4)
                value = obj.getProperty(5)
 
                red, green, blue = [int((x / 255.0) * bri) for x in HSV_2_RGB((hue, saturation, value))]
                grove_set_color(self.myLcd, red, green, blue)
                grove_set_text(self.myLcd, str(display_value))
                print "display value: %d" % display_value
            else:
                if not self.cleanup:
                    grove_set_color(self.myLcd, 0, 0, 0)
                    grove_clear(self.myLcd)
                    print "clear display"
                    self.cleanup = True
            self.previous = on_off

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):

            self.m1 = Grove_LCD()
            self.addClass(self.m1,0)
            self.obj_grove_lcd = self.addObject(self.m1.ID)


    if len(sys.argv) <= 2:
        print 'python %s <gip> <dip>:<port>' % sys.argv[0]
        print '      <gip>: IP addrees of gateway'
        print '      <dip>: IP address of Python device'
        print '      <port>: An unique port number'
        print ' ex. python %s 192.168.4.7 127.0.0.1:3000' % sys.argv[0]
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
