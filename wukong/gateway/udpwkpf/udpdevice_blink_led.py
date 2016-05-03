from twisted.internet import reactor
from udpwkpf import WuClass, Device
import sys
from udpwkpf_io_interface import *

Light_Actuator_Pin = 13 
Button_Pin = 5 

if __name__ == "__main__":
    class Button(WuClass):
        def __init__(self):
            self.ID = 1012

        def setup(self, obj, pin):
            button_gpio = pin_mode(pin, PIN_TYPE_DIGITAL, PIN_MODE_INPUT)
            print "Button init success"
            reactor.callLater(0.5, self.refresh, obj, pin, button_gpio)

        def refresh(self, obj, pin, button_gpio):
            current_value = digital_read(button_gpio)
            obj.setProperty(0, current_value)
            print "Button pin: ", pin, ", value: ", current_value
            reactor.callLater(0.5, self.refresh, obj, pin, button_gpio)

    class Light_Actuator(WuClass):
        def __init__(self, pin):
            self.ID = 2001
            self.light_actuator_gpio = pin_mode(pin, PIN_TYPE_DIGITAL, PIN_MODE_OUTPUT)
            print "Light Actuator init success"

        def update(self,obj,pID,val):
            if pID == 0:
                if val == True:
                    digital_write(self.light_actuator_gpio, 1)
                    print "Light Actuator On"
                else:
                    digital_write(self.light_actuator_gpio, 0)
                    print "Light Actuator Off"
            else:
                print "Light Actuator garbage"

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
            m1 = Light_Actuator(Light_Actuator_Pin)
            self.addClass(m1,0)
            self.obj_light_actuator = self.addObject(m1.ID)

            m2 = Button()
            self.addClass(m2,0)
            self.obj_button = self.addObject(m2.ID)
            self.obj_button.cls.setup(self.obj_button, Button_Pin)

    if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python <filename> <gateway ip> <local ip>:<any given port number>'
        print ' ex. python udpdevice_grove_kit_sample.py 192.168.4.7 127.0.0.1:3000'
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
    device_cleanup()
