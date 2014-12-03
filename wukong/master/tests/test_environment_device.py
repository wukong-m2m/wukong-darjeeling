# vim: ts=4 sw=4
import os,sys,time
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from wkpf.models import *
from wkpf.locationTree import *
from wkpf.parser import *
from configuration import *
from wkpf.wkpfcomm import *
from wkpf.wuclasslibraryparser import *
from wkpf.wuapplication import *
import serial

from threading import Thread

class WuTest:
    def __init__(self, download=True, pair=True):
        ## set up FBP application
        self.application = None
        #self.loadApplication(APP_PATH)

        ## set up manageable devices
        self.devs = TEST_DEVICES
        self.hexfiles = HEXFILES
        assert len(self.devs) == len(self.hexfiles)
        self.dev_len = len(self.devs)
        if download is True:
            self.__downloadAll()

        ## load wukong component library
        WuClassLibraryParser.read(COMPONENTXML_PATH)


        ## set up communication gateway
        self.comm = getComm()

        self.node_ids = [i + 2 for i in xrange(self.dev_len)]

        ## start the device consoles
        self.consoles = {}
        for dev in self.devs:
            self.consoles[dev] = serial.Serial(dev, baudrate=115200)
            self.consoles[dev].timeout = 1

        if pair is True:
            self.pair_devices_gateway()

    def pair_devices_gateway(self):
        self.constrollerReset()

        for dev in self.devs:
            self.stop()
            ret = self.wait('done')
            self.add()
            ret = self.wait('ready')
            print "----->", ret
            self.waitDeviceReady(dev)
            #self.deviceReset(dev)
            self.deviceLearn(dev)
            self.wait('found',20)

    ### We may assume only one application in one WuTest object.
    def loadApplication(self, dir_path):
        self.application = WuApplication(dir=dir_path)
        self.application.loadConfig()

    def mapping(self, nodes_info):
        if self.application is not None:
            self.application.parseApplication()
            locTree = LocationTree(LOCATION_ROOT)
            locTree.buildTree(nodes_info)
            self.application.mapping(locTree, self.comm.getRoutingInformation())

    def deploy_with_discovery(self):
        if self.application is not None:
            self.application.deploy_with_discovery(['arduino'])

    def __downloadAll(self):
        for i in xrange(len(self.devs)):
            self.__download(self.devs[i], self.hexfiles[i])

    def __download(self, port_name, hexfile):
        os.system("avrdude -p atmega2560 -c wiring -P %s -U flash:w:%s" % (port_name, hexfile))

    def add(self):
        self.comm.onAddMode()
    
    def delete(self):
        self.comm.onDeleteMode()

    def stop(self):
        self.comm.onStopMode()

    def wait(self, findStr, timeout=5):
        print "wait for ", findStr
	
        while timeout > 0:
            st = self.comm.currentStatus()
	    print "--->",st
            if st.find(findStr) != -1:
		print "matched"
                return True
            time.sleep(1)
            timeout = timeout - 1
        return False

    def waitDeviceReady(self, dev, timeout=20):
        while timeout > 0:
            timeout = timeout - 1
            l = self.consoles[dev].readline()
	    print "-->",l
            if l.find("ready") != -1: break
	print " device ready"

    def deviceLearn(self, dev):
        self.consoles[dev].write("$l")

    def deviceReset(self, dev):
        self.consoles[dev].write("$r")

    def deviceWait(self, dev):
        self.consoles[dev].write("$w")

    def deviceResume(self, dev):
        self.consoles[dev].write("#")

    def constrollerReset(self):
        command = '../../tools/testrtt/a.out -d %s nowait controller reset' % (ZWAVE_GATEWAY_IP)
        os.system(command)
        time.sleep(1)

    def discovery(self, force=True):
        self.allNodesInfos = self.comm.getAllNodeInfos(force, 'nodes.xml')
        return self.allNodesInfos

    def setLocation(self, node_id, location):
        self.comm.setLocation(node_id, location)

    def getLocation(self, node_id):
        return self.comm.getLocation(node_id)

    def getProperty(self, node_id, port, wuclassid, property_number):
        res = self.comm.getProperty(node_id, port, wuclassid, property_number)
        # res = [value, datatype, status]
	print res
        value = res[0]
        return value

    def setProperty(self, node_id, port, wuclassid, property_number, datatype, value):
        self.comm.setProperty(node_id, port, wuclassid, property_number, datatype, value)

    def getWuClassList(self, node_id):
        return self.comm.getWuClassList(node_id)

    def getWuObjectList(self, node_id):
        return self.comm.getWuObjectList(node_id)

    def getNodeInfo(self, node_id):
        self.comm.getNodeInfo(node_id)

    def countWuObjectByWuClassID(self, wuClassID):
        cnt = 0
        for node in self.allNodesInfos:
            for key in node.wuobjects:
                wuobject = node.wuobjects[key]
                if wuClassID == wuobject.wuclassdef.id:
                    cnt += 1

        return cnt

    def start_log(self):
        def func(self, console, filename):
            f = open(filename, 'w')

            while not self.log_done:
                s = console.readline()
                f.write(s)
                f.flush()
            f.close()

        self.log_done = False
        self.log_threads = []
        for dev in self.devs:
            filename = 'reports/node_%d.txt' % (2)
            thread = Thread(target=func, args=(self, self.consoles[dev], filename))
            thread.start()
            self.log_threads.append(thread)

    def stop_log(self):
        time.sleep(1)
        self.log_done = True
        for thread in self.log_threads:
            thread.join()



if __name__ == '__main__':
    test = WuTest(True, True)
    #test = WuTest(False, False)
    
    nodes_info = test.discovery()
    #
    #print 'nodes:', nodes_info
    #node = nodes_info[0]
    #print 'node:', node.id, node.wuobjects
    #for port in node.wuobjects:
    #    print 'port:', port
    #    wuclass = node.wuobjects[port].wuclassdef
    #    print 'obj:', wuclass.id, wuclass.name
    
    #test.loadApplication(APP_PATH) 
    #test.mapping(nodes_info)
    #test.deploy_with_discovery()
