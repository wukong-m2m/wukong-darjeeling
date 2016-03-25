from udpwkpf import WuClass, Device
import sys
import mraa

from twisted.protocols import basic
from twisted.internet import reactor, protocol

if __name__ == "__main__":
    class Threshold(WuClass):
        def __init__(self):
            self.ID = 1
            print "Threshold init success"
        def update(self,obj,pID,val):
            print "Threshold input value: ", val
            if pID == 2:
                op = obj.getProperty(0)
                if op == 0:
                    if val < obj.getProperty(1):
                        print "op= ", op, " True"
                        obj.setProperty(3,True)
                    else:
                        print "op= ", op, " False"
                        obj.setProperty(3,False)
                elif op == 1:
                    if val > obj.getProperty(1):
                        print "op= ", op, " True"
                        obj.setProperty(3,True)
                    else:
                        print "op= ", op, " False"
                        obj.setProperty(3,False)
                elif op == 2:
                    if val <= obj.getProperty(1):
                        print "op= ", op, " True"
                        obj.setProperty(3,True)
                    else:
                        print "op= ", op, " False"
                        obj.setProperty(3,False)
                elif op == 3:
                    if val >= obj.getProperty(1):
                        print "op= ", op, " True"
                        obj.setProperty(3,True)
                    else:
                        print "op= ", op, " False"
                        obj.setProperty(3,False)
                else:
                    print "Error: unknown operator %d" % op
            pass
        def init(self):
            pass

    class Slider(WuClass):
        def __init__(self):
            self.ID = 1006
        
        def setup(self, obj, pin):
            slider_aio = mraa.Aio(pin)
            print "Slider init success"
            reactor.callLater(0.1, self.refresh, obj, pin, slider_aio)

        def refresh(self, obj, pin, slider_aio):
            current_value = slider_aio.read() / 4
            obj.setProperty(2, current_value)
            print "Slider pin: ", pin, ", value: ", current_value
            reactor.callLater(0.1, self.refresh, obj, pin, slider_aio)
            
    class Light_Actuator(WuClass):
        def __init__(self, pin):
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

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
            Light_Actuator_Pin = 2 
            m1 = Light_Actuator(Light_Actuator_Pin)
            self.addClass(m1,0)
            self.obj_light_actuator = self.addObject(m1.ID)

            m2 = Slider()
            self.addClass(m2,0)
            self.obj_slider = self.addObject(m2.ID)
            self.obj_slider.cls.setup(self.obj_slider, 0)

            m3 = Threshold()
            self.addClass(m3,1)
            self.obj_threshold = self.addObject(m3.ID)

    if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python <filename> <gateway ip> <local ip>:<any given port number>'
        print ' ex. python udpdevice_grove_kit_sample.py 192.168.4.7 127.0.0.1:3000'
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
