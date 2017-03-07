from udpwkpf import WuClass, Device
import sys
from udpwkpf_io_interface import *
from twisted.internet import reactor

Light_Sensor_Pin = 1

if __name__ == "__main__":
    class Light_Sensor(WuClass):
        def __init__(self):
            WuClass.__init__(self)
            self.loadClass('Light_Sensor')
            self.light_sensor_aio = pin_mode(Light_Sensor_Pin, PIN_TYPE_ANALOG)

        def update(self,obj,pID=None,val=None):
            try:
                current_value = analog_read(self.light_sensor_aio)/4 # 4 is divisor value which depends on the light sensor
                obj.setProperty(0, current_value)
                print "Light sensor analog pin: ", Light_Sensor_Pin, ", value: ", current_value
            except IOError:
                print ("Error")

    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
            cls = Light_Sensor()
            self.addClass(cls,0)
            self.obj_light_sensor = self.addObject(cls.ID)

    if len(sys.argv) <= 2:
        print 'python %s <gip> <dip>:<port>' % sys.argv[0]
        print '      <gip>: IP addrees of gateway'
        print '      <dip>: IP address of Python device'
        print '      <port>: An unique port number'
        print ' ex. python %s 192.168.4.7 127.0.0.1:3000' % sys.argv[0]
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
