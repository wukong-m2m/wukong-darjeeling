import traceback
import threading
import time,sys
from udpwkpf import WuClass, Device
from twisted.internet import reactor
import random
from math import log
#import pyupm_lpd8806
from bootstrap import *
import time

class TurnOnLED(object):
    def __init__(self):
        save       = [[1,2,4,7,9,10]]
        turn_right = [[],[7],[0,6,8],[1,5,9],[2,4,10],[3,11]]
        turn_left  = [[],[4],[3,5,11],[2,6,10],[1,7,9],[0,8]]
        stay       = [[0,3,5,6,8,11]]
        self.pattern = [save, turn_left, turn_right, stay]
        self.lock = threading.Lock()        
    def show(self, e, index, r, g, b, step, maxLevel):
        print "Index %d waiting for a lock" % (index)
        self.lock.acquire()
        while not e.isSet():  
          for i in range(len(self.pattern[index])):
            event_is_set = e.wait(0)
            if event_is_set:
              break
            else:
	      level = 0.1
	      dir = step
	      while level > 0.0:
                for j in range(len(self.pattern[index][i])):
	          if(level > 0.0 and level < maxLevel):   
                    led.set(self.pattern[index][i][j],Color(r,g,b,level))
                led.update()
                if(level >= (maxLevel - 0.1)):
	          dir = -step 
                level += dir
        self.lock.release()
        print "Index %d release a lock" % (index)

class Pattern(WuClass):
    def __init__(self):
        WuClass.__init__(self)
        self.loadClass('Pattern')
        self.led = TurnOnLED()
	self.e = threading.Event()
	t = threading.Thread(target=self.worker, args=(self.led, self.e, 0, 0, 255, 0, 0.05, 0.7,))
	t.start()
	print "Pattern 0 start"

    def update(self,obj,pID,value):
        self.e.set()
        index = obj.getProperty(0)
        if index == 0:
          self.e = threading.Event()
	  t = threading.Thread(target=self.worker, args=(self.led, self.e, index, 0, 255, 0, 0.05, 0.7,))
	  t.start()
	  print "Pattern 0 start"
        elif index == 1:
          self.e = threading.Event()
	  t = threading.Thread(target=self.worker, args=(self.led, self.e, index, 255, 0, 0, 0.3, 0.7,))
	  t.start()
	  print "Pattern 1 start"
        elif index == 2:
          self.e = threading.Event()
	  t = threading.Thread(target=self.worker, args=(self.led, self.e, index, 255, 0, 0, 0.3, 0.7,))
	  t.start()
	  print "Pattern 2 start"
        elif index == 3:
          self.e = threading.Event()
	  t = threading.Thread(target=self.worker, args=(self.led, self.e, index, 255, 0, 0, 0.3, 0.7,))
	  t.start()
	  print "Pattern 3 start"
        elif index == 4:
	  print "Turn off pattern"

    def worker(self, led, e, index, r, g, b, step, maxLevel):
        led.show(e, index, r, g, b, step, maxLevel)
        print "Done index: ", index
        
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

