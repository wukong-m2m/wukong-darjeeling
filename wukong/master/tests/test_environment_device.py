# vim: ts=4 sw=4
import os,sys,time
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from wkpf.models import *
from wkpf.locationTree import *
from wkpf.parser import *
from configuration import *
from wkpf.wkpfcomm import *
from wkpf.wuclasslibraryparser import *
import serial

class WuTest:
    def __init__(self, dev):
        self.dev = dev
        self.comm = getComm()
        WuClassLibraryParser.read(COMPONENTXML_PATH)

    def download(self, hexfile):
        os.system("avrdude -p atmega2560 -c wiring -P %s -U flash:w:%s" % (self.dev, hexfile))
    def add(self):
        self.comm.onAddMode()
    def stop(self):
        self.comm.onStopMode()
    def wait(self,findStr,timeout=5):
        print timeout
        while timeout > 0:
            st = self.comm.currentStatus()
            if st.find(findStr) != -1:
                return True
            time.sleep(1)
            timeout = timeout - 1
        return False
    def waitDeviceReady(self,timeout=20):
        self.console = serial.Serial(self.dev,baudrate=115200)
        while timeout > 0:
            timeout = timeout - 1
            l = self.console.readline()
            if l.find("ready") != -1: break
    def deviceLearn(self):
        self.console.write("$l")
    def deviceReset(self):
        self.console.write("$r")
    def constrollerReset(self):
        command = '../../tools/testrtt/a.out -d %s nowait controller reset' % (ZWAVE_GATEWAY_IP)
        os.system(command)

    def discovery(self):
        self.comm.getNodeIds()
        self.comm.getAllNodeInfos(False)


def initDevice(dev):
    obj = WuTest(dev)
    return obj



if __name__ == '__main__':
    dev = initDevice('/dev/cu.usbserial-A96P9ZJF')
    #dev.download('vm1.hex')
    dev.stop()
    ret = dev.wait('done')
    dev.add()
    ret = dev.wait('ready')
    print "----->", ret
    dev.waitDeviceReady()
    dev.deviceReset()
    dev.deviceLearn()
    dev.wait('found',20)

