import time,sys
from udpwkpf import WuClass, Device
from udpwkpf_io_interface import *
from twisted.internet import reactor
import random
from math import log
import time

import udpdevice_pattern_no_thread
import udpdevice_fire_agent
import udpdevice_smoke_sensor
import udpdevice_counter

class MyDevice(Device):
    def __init__(self,addr,localaddr):
        Device.__init__(self,addr,localaddr)

    def init(self):
        m = udpdevice_pattern_no_thread.Pattern()
        self.addClass(m,1)
        self.obj_test = self.addObject(m.ID)
        m2 = udpdevice_fire_agent.Fire_Agent()
        self.addClass(m2, self.FLAG_VIRTUAL)
        self.obj_fire_agent = self.addObject(m2.ID)
        m3 = udpdevice_smoke_sensor.Smoke_Sensor()
        self.addClass(m3,0)
        self.obj_smoke_sensor = self.addObject(m3.ID)
        m4 = udpdevice_counter.Counter()
        self.addClass(m4,0)
        self.obj_counter = self.addObject(m4.ID)

if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <ip:port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python udpwkpf.py 127.0.0.1 3000'
        sys.exit(-1)

d = MyDevice(sys.argv[1],sys.argv[2])

reactor.run()

