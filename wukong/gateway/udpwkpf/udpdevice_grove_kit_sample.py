from twisted.internet import reactor
from udpwkpf import WuClass, Device
import sys
from udpwkpf_io_interface import *

Slider_Pin = 0
Light_Actuator_Pin = 7

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

    class Slider(WuClass):
        def __init__(self):
            WuClass.__init__(self)
            self.loadClass('Slider')
            self.slider_aio = pin_mode(Slider_Pin, PIN_TYPE_ANALOG)
            print "Slider init success"

        def update(self,obj,pID=None,val=None):
            try:
                current_value = analog_read(self.slider_aio)/4 #4 is divisor value which depends on the slider.
                obj.setProperty(2, current_value)
                print "Slider analog pin: ", Slider_Pin, ", value: ", current_value
            except IOError:
                print ("Error")

    class Light_Actuator(WuClass):
        def __init__(self):
            WuClass.__init__(self)
            self.loadClass('Light_Actuator')
            self.light_actuator_gpio = pin_mode(Light_Actuator_Pin, PIN_TYPE_DIGITAL, PIN_MODE_OUTPUT)

        def update(self,obj,pID=None,val=None):
            try:
                if pID == 0:
                    if val == True:
                        digital_write(self.light_actuator_gpio, 1)
                        print "Light Actuator On"
                    else:
                        digital_write(self.light_actuator_gpio, 0)
                        print "Light Actuator Off"
            except IOError:
                print ("Error")

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):

            m1 = Light_Actuator()
            self.addClass(m1,0)
            self.obj_light_actuator = self.addObject(m1.ID)

            m2 = Slider()
            self.addClass(m2,0)
            self.obj_slider = self.addObject(m2.ID)

            m3 = Threshold()
            self.addClass(m3,self.FLAG_VIRTUAL)
            self.obj_threshold = self.addObject(m3.ID)

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