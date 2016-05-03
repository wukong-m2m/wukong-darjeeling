from twisted.internet import reactor
from udpwkpf import WuClass, Device
import sys
import mraa
from twisted.protocols import basic
from twisted.internet import reactor, protocol

import pyupm_i2clcd as lcd

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
            self.ID = 2010
	    self.myLcd=lcd.Jhd1313m1(0,0x3E,0x62)
            print "LCD Actuator init success"

        def update(self,obj,pID,val):
            display_value = obj.getProperty(0)
            hue = obj.getProperty(1)
            saturation = obj.getProperty(2)
            value = obj.getProperty(3)
            red, green, blue = HSV_2_RGB((hue, saturation, value))
            self.myLcd.setColor( red, green, blue)
            self.myLcd.setCursor(0,0)
            self.myLcd.write(str(display_value))
            print "display value: %d" % display_value

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
			
            self.m1 = Grove_LCD()
            self.addClass(self.m1,0)
            self.obj_grove_lcd = self.addObject(self.m1.ID)
				

    if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python udpwkpf.py <gateway IP> <local IP>:3000'
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
