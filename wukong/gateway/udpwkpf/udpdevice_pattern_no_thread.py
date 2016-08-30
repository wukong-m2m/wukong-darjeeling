import traceback
import threading
import time,sys
from udpwkpf import WuClass, Device
from udpwkpf_io_interface import *
from twisted.internet import reactor
import random
from math import log
import time

class Pattern(WuClass):
    def __init__(self):
        WuClass.__init__(self)
        self.loadClass('Pattern')
        save       = [[7],[1,9],[2,10],[4],[7],[1,9],[2,10],[4],[7],[1,9],[2,10],[4],[7],[1,9],[2,10],[4],[7],[1,9],[2,10],[4],[7],[1,9],[2,10],[4]]
        turn_right = [[],[],[],[],[7],[],[],[],[0,8],[6],[],[],[],[1,9],[5],[],[],[],[2,10],[4],[],[],[],[3,11]]
        turn_left  = [[],[],[],[],[],[],[],[4],[],[],[5],[3,11],[],[6],[2,10],[],[7],[1,9],[],[],[0,8],[],[],[]]
        stay       = [[0,8],[6],[5],[3,11],[0,8],[6],[5],[3,11],[0,8],[6],[5],[3,11],[0,8],[6],[5],[3,11],[0,8],[6],[5],[3,11],[0,8],[6],[5],[3,11]]
	clean      = [[0,7,8],[1,6,9],[2,5,10],[3,4,11]]
        self.pattern = [save, turn_right, turn_left, stay, clean]
	self.count = 0
 	init_pattern = [7,1,9,2,10,4]

        if device_type == DEVICE_TYPE_MRAA:
          self.ledNUM = 12
          self.mystrip = pyupm_lpd8806.LPD8806(self.ledNUM, 7)
	  #turn off all led with edison's API
	  for i in range(self.ledNUM):
            self.mystrip.setPixelColor(i, 0, 0, 0)
          self.mystrip.show()
	  #turn on init pattern with edison's API
	  for i in range(len(init_pattern)):
	    self.mystrip.setPixelColor(init_pattern[i], 0, 127, 0)
          self.mystrip.show()
	elif device_type == DEVICE_TYPE_RPI:
          led.all_off()
	  #turn on init pattern with Pi's API
	  for i in range(len(init_pattern)):
	    led.set(init_pattern[i],Color(0, 255, 0, 0.5))
	  led.update()
	print "Init: Pattern 0 start" 

    def update(self,obj,pID,value):
 	index = obj.getProperty(0)
        if device_type == DEVICE_TYPE_MRAA:
          step, maxLevel = [0.02, 0.5]
	elif device_type == DEVICE_TYPE_RPI:
          step, maxLevel = [0.1, 0.5]
	self.clean(self.count)
        if index == 0:	
	  pattern_slice = self.pattern[index][self.count]
	  self.show(pattern_slice, 0, 255, 0, step, maxLevel)
	  print "Pattern 0 start"
        elif index == 1:
	  pattern_slice = self.pattern[index][self.count]
	  self.show(pattern_slice, 255, 127, 39, step, maxLevel)
	  print "Pattern 1 start"
        elif index == 2:
	  pattern_slice = self.pattern[index][self.count]
	  self.show(pattern_slice, 0, 162, 232, step, maxLevel)
	  print "Pattern 2 start"
        elif index == 3:
	  pattern_slice = self.pattern[index][self.count]
	  self.show(pattern_slice, 255, 0, 0, step, maxLevel)
	  print "Pattern 3 start"
        elif index == 4:
	  print "Clean pattern"
          pass	
        self.count = (self.count + 1) % 20 # 20 is the number of pattern slice

    def show(self, pattern_slice, r, g, b, step, maxLevel):
        level = 0.0
        while level <= maxLevel:
          if device_type == DEVICE_TYPE_MRAA:
            for i in range(len(pattern_slice)):  
              self.mystrip.setPixelColor(pattern_slice[i], int(r*level), int(g*level), int(b*level))
            self.mystrip.show()
	  elif device_type == DEVICE_TYPE_RPI:
            for i in range(len(pattern_slice)):   
              led.set(pattern_slice[i],Color(r,g,b,level))
            led.update()
          level += step

    def clean(self, count):
	count = count % 4
        if device_type == DEVICE_TYPE_MRAA:
	  for i in range(3):
            self.mystrip.setPixelColor(self.pattern[4][count][i], 0, 0, 0)
          self.mystrip.show()
	elif device_type == DEVICE_TYPE_RPI:
	  for i in range(3):
            led.set(self.pattern[4][count][i], Color(0, 0, 0, 0))
          led.update()

if __name__ == "__main__":
    class MyDevice(Device):
        def __init__(self,addr,localaddr):
            Device.__init__(self,addr,localaddr)

        def init(self):
            m = Pattern()
            self.addClass(m,1)
            self.obj_test = self.addObject(m.ID)
    
    if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <ip:port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python udpwkpf.py 127.0.0.1 3000'
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])

    reactor.run()

