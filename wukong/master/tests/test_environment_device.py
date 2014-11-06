#!/usr/bin/python
# vim: ts=4 sw=4
import os,sys,time
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from wkpf.model.models import *
from wkpf.model.locationTree import *
from wkpf.parser import *
from configuration import *
from wkpf.wkpfcomm import *
from wkpf.wuclasslibraryparser import *
from wkpf.wuapplication import *
from device_handlers import *
import serial



# For Galileo/NetworkServer
TEST_DEVICES = [
    Galileo_NetworkServer(binary='../../../src/config/galileo', ipaddress='192.168.4.111'),
    Galileo_NetworkServer(binary='../../../src/config/galileo', ipaddress='192.168.4.116'),
    Galileo_NetworkServer(binary='../../../src/config/galileo', ipaddress='192.168.4.117')
]

# For WuDevice/ZWave
# TEST_DEVICES = [
#     WuDevice_Zwave(binary='arduino/wukong_test2_no_watchdog.cpp.hex', usbport='/dev/ttyUSB0'),
#     WuDevice_Zwave(binary='arduino/wukong_test2_no_watchdog.cpp.hex', usbport='/dev/ttyUSB1'),
#     WuDevice_Zwave(binary='arduino/wukong_test2_no_watchdog.cpp.hex', usbport='/dev/ttyUSB2')
# ]



class WuTest:
    def __init__(self, download=True, pair=True):
        ## set up FBP application
        self.application = None
        #self.loadApplication(APP_PATH)

	#os.system("kill -9  `cat /tmp/gateway.pid`;rm /tmp/gateway.pid;sleep 2")
        ## set up manageable devices
        self.devs = TEST_DEVICES
        if download is True:
            for dev in self.devs:
                dev.download()

        ## load wukong component library
        WuClassLibraryParser.read(COMPONENTXML_PATH)

        ## set up communication gateway

        ## set node_ids. This assumes all zwave nodes will be numbered from 2 up. Network server nodes use the last byte of their IP as node_id
        next_zwave_id = 2
        for dev in self.devs:
            if dev.__class__ == WuDevice_Zwave:
                dev.node_id = next_zwave_id
                next_zwave_id += 1

        if pair is True:
            self.pair_devices_gateway()

    def pair_devices_gateway(self):
        self.constrollerReset()
        self.comm = getComm()
	gevent.sleep(4)
	os.system("rm PAUSE_GATEWAY")
	print "%"*100
	gevent.sleep(10)

        for dev in self.devs:
            self.stop()
            ret = self.wait('done')
            self.add()
            ret = self.wait('ready')
            print "----->", ret
            dev.waitDeviceReady()
            #dev.deviceReset()
            dev.deviceLearn()
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

    def add(self):
        self.comm.onAddMode()

    def stop(self):
        self.comm.onStopMode()

    def wait(self, findStr, timeout=5):
        print timeout
        while timeout > 0:
            st = self.comm.currentStatus()
            if st.find(findStr) != -1:
                return True
            gevent.sleep(1)
            timeout = timeout - 1
        return False

    def constrollerReset(self):
        command = '../../tools/testrtt/a.out -d %s nowait controller reset' % (ZWAVE_GATEWAY_IP)
        os.system(command)
        gevent.sleep(1)

    def discovery(self, force=True):
        self.allNodesInfos = self.comm.getAllNodeInfos(force)
        return self.allNodesInfos

    def setLocation(self, node_id, location):
        self.comm.setLocation(node_id, location)

    def getLocation(self, node_id):
        return self.comm.getLocation(node_id)

    def getProperty(self, node_id, port, wuclassid, property_number):
        res = self.comm.getProperty(node_id, port, wuclassid, property_number)
        # res = [value, datatype, status]
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
        for dev in self.devs:
            dev.startLog()

    def stop_log(self):
        for dev in self.devs:
            dev.stopLog()



if __name__ == '__main__':
    test = WuTest(False, True)

    # nodes_info = test.discovery()

    # test.loadApplication(APP_PATH) 
    # test.mapping(nodes_info)
    # test.deploy_with_discovery()
