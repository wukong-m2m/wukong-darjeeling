from udpwkpf import WuClass, Device
import sys

from twisted.protocols import basic
from twisted.internet import reactor, protocol

import pyaudio
import wave
import os

if __name__ == "__main__":
    class Intel_Sound(WuClass):
        def __init__(self):
            self.ID = 2037
        def update(self,obj,pID,val):
            if val:
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

    if len(sys.argv) <= 2:
        print 'python udpwkpf.py <ip> <port>'
        print '      <ip>: IP of the interface'
        print '      <port>: The unique port number in the interface'
        print ' ex. python <filename> <gateway ip> <local ip>:<any given port number>'
        print ' ex. python udpdevice_grove_kit_sample.py 192.168.4.7 127.0.0.1:3000'
        sys.exit(-1)

    d = MyDevice(sys.argv[1],sys.argv[2])
    reactor.run()
