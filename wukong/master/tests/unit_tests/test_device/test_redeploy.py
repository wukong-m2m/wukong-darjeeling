import os, sys
import unittest
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../..'))
from test_environment_device import WuTest
from configuration import *
import gevent

class TestRedeploy(unittest.TestCase):
    def setUp(self):
        self.test = WuTest(False, False)

    def test_application(self):
	self.test.deviceResume(DEVICE1)
	self.test.deviceResume(DEVICE2)
	self.test.deviceResume(DEVICE3)
	nodes_info = self.test.discovery()
	self.test.setLocation(3, "/WuKong/binary")
	self.test.setLocation(2, "/WuKong/sound")
	self.test.setLocation(4, "/WuKong/sound")
        
	nodes_info = self.test.discovery()
        self.test.loadApplication("applications/redeploy_one_to_one")
    	self.test.mapping(nodes_info)
    	self.test.deploy_with_discovery()
	gevent.sleep(10)
        
	slider_node_id = 3
        slider_port = 2
        slider_wuclass_id = 15
        slider_property_number = 2
        slider_datatype = 'short'
        slider_value = 50
        self.test.setProperty(slider_node_id, slider_port, slider_wuclass_id, slider_property_number, slider_datatype, slider_value)

        binary_sensor_node_id = 3
        binary_sensor_port = 1
        binary_sensor_wuclass_id = 10
        binary_property_number = 1
        binary_datatype = 'boolean'
	ans = 0
	self.test.setProperty(binary_sensor_node_id, binary_sensor_port, binary_sensor_wuclass_id, binary_property_number, binary_datatype, ans)

        
        sound_node_id = 2
        sound_port = 3
        sound_wuclass_id = 204
        sound_property_number = 0
        
	ans = 1
	self.test.setProperty(binary_sensor_node_id, binary_sensor_port, binary_sensor_wuclass_id, binary_property_number, binary_datatype, ans)

    	for loop in range(0,11):
	    gevent.sleep(0.1)
            res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
	    if res == (ans == 1):
	        break
        self.assertNotEqual(loop, 10)
	
	self.test.deviceWait(DEVICE1)
        
	nodes_info = self.test.discovery()
        self.test.loadApplication("applications/redeploy_one_to_one")
    	self.test.mapping(nodes_info)
    	self.test.deploy_with_discovery()
	gevent.sleep(10)
        
        sound_node_id = 4
	ans = 0
	self.test.setProperty(binary_sensor_node_id, binary_sensor_port, binary_sensor_wuclass_id, binary_property_number, binary_datatype, ans)

    	for loop in range(0,11):
	    gevent.sleep(0.1)
            res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
	    if res == (ans == 1):
	        break
        self.assertNotEqual(loop, 10)
        
	ans = 1
	self.test.setProperty(binary_sensor_node_id, binary_sensor_port, binary_sensor_wuclass_id, binary_property_number, binary_datatype, ans)


    	for loop in range(0,11):
	    gevent.sleep(0.1)
            res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
	    if res == (ans == 1):
	        break
        self.assertNotEqual(loop, 10)

	self.test.deviceResume(DEVICE1)
		
    def test_super_application(self):
	self.test.deviceResume(DEVICE1)
	self.test.deviceResume(DEVICE2)
	self.test.deviceResume(DEVICE3)
        nodes_info = self.test.discovery()
	self.test.setLocation(3, "/WuKong/binary")
	self.test.setLocation(2, "/WuKong/sound")
	self.test.setLocation(4, "/WuKong/sound")

	slider_node_id = 3
	slider_port = 2
	slider_wuclass_id = 15
	slider_property_number = 2
	slider_datatype = 'short'
	slider_value = 50
	self.test.setProperty(slider_node_id, slider_port, slider_wuclass_id, slider_property_number, slider_datatype, slider_value)

	binary_sensor_node_id = 3
	binary_sensor_port = 1
	binary_sensor_wuclass_id = 10
	binary_property_number = 1
	binary_datatype = 'boolean'
	ans = 0
	self.test.setProperty(binary_sensor_node_id, binary_sensor_port, binary_sensor_wuclass_id, binary_property_number, binary_datatype, ans)

	sound_node_id = 2
	sound_port = 3
	sound_wuclass_id = 204
	sound_property_number = 0
	
	
        for i in xrange(10):
            print "Redeploy loop:%d" % i
            if i%2 == 0:
	        self.test.deviceWait(DEVICE1)
		sound_node_id = 4
            else:
	        self.test.deviceResume(DEVICE1)
		gevent.sleep(3)
		sound_node_id = 2

            nodes_info = self.test.discovery()
            self.test.loadApplication("applications/redeploy_one_to_one")
            self.test.mapping(nodes_info)
            self.test.deploy_with_discovery()
	    gevent.sleep(10)

	    ans = 0
	    self.test.setProperty(binary_sensor_node_id, binary_sensor_port, binary_sensor_wuclass_id, binary_property_number, binary_datatype, ans)
	    for loop in range(0,11):
		gevent.sleep(0.1)
		res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
	        if res == (ans == 1):
	            break
            self.assertNotEqual(loop, 10)
	
	    ans = 1
	    self.test.setProperty(binary_sensor_node_id, binary_sensor_port, binary_sensor_wuclass_id, binary_property_number, binary_datatype, ans)

	    for loop in range(0,11):
		gevent.sleep(0.1)
		res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
	        if res == (ans == 1):
	            break
            self.assertNotEqual(loop, 10)
	

if __name__ == '__main__':
    unittest.main()

