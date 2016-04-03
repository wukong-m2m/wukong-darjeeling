from udpwkpf import WuClass, Device
import sys
import mraa

from twisted.protocols import basic
from twisted.internet import reactor, protocol

if __name__ == "__main__":
    class Light_Sensor(WuClass):
        def __init__(self):
            self.ID = 1001
        
        def setup(self, obj, pin):
            light_sensor_aio = mraa.Aio(pin)
            print "Light sensor init success"
            reactor.callLater(0.1, self.refresh, obj, pin, light_sensor_aio)

        def refresh(self, obj, pin, light_sensor_aio):
            current_value = light_sensor_aio.read() / 4
            obj.setProperty(2, current_value)
            print "Light sensor pin: ", pin, ", value: ", current_value
            reactor.callLater(0.1, self.refresh, obj, pin, light_sensor_aio)
            
    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):

            cls = Light_Sensor()
            self.addClass(cls,0)
            self.obj_light_sensor = self.addObject(cls.ID)
            self.obj_light_sensor.cls.setup(self.obj_light_senosr, 0)

    if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python <filename> <gateway ip> <local ip>:<any given port number>'
        print ' ex. python udpdevice_grove_kit_sample.py 192.168.4.7 127.0.0.1:3000'
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
