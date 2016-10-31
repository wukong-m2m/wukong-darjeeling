from twisted.internet import reactor
from udpwkpf import WuClass, Device
import sys

import pyaudio
import wave
import os

class Intel_Sound(WuClass):
    def __init__(self):
        WuClass.__init__(self)
        self.loadClass('Intel_Sound')
    def update(self,obj,pID=None,val=None):
        on_off = obj.getProperty(0)
        if on_off:
            path = os.path.abspath("intel.wav")
            chunk = 1024
            wf = wave.open(path, 'rb')
            p = pyaudio.PyAudio()

            stream = p.open(
                format = p.get_format_from_width(wf.getsampwidth()),
                channels = wf.getnchannels(),
                rate = wf.getframerate(),
                output = True)
            data = wf.readframes(chunk)

            while data != '':
                stream.write(data)
                data = wf.readframes(chunk)

            stream.close()
            p.terminate()
        else:
            pass

class MyDevice(Device):
    def __init__(self,addr,localaddr):
        Device.__init__(self,addr,localaddr)

    def init(self):
        m1 = Intel_Sound()
        self.addClass(m1,0)
        self.obj_intel_sound = self.addObject(m1.ID)

if __name__ == "__main__":
    if len(sys.argv) <= 2:
        print 'python %s <gip> <dip>:<port>' % sys.argv[0]
        print '      <gip>: IP addrees of gateway'
        print '      <dip>: IP address of Python device'
        print '      <port>: An unique port number'
        print ' ex. python %s 192.168.4.7 127.0.0.1:3000' % sys.argv[0]
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
