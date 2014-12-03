import os, sys ,gevent
import unittest
import random
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../..'))
from test_environment_device import WuTest
from configuration import *

class TestPropagate(unittest.TestCase):
    def setUp(self):
        self.test = WuTest(False, False)

    def test_basic_propagate(self):        
        nodes_info = self.test.discovery()
#	self.test.setLocation(3, "/WuKong/slider")
#	self.test.setLocation(2, "/WuKong/binary")
#	self.test.setLocation(4, "/WuKong/sound")
        self.test.loadApplication("applications/three_components_in_one_device") 
    	self.test.mapping(nodes_info)
    	self.test.deploy_with_discovery()
	gevent.sleep(10)

        slider_node_id = 2
        slider_port = 2
        slider_wuclass_id = 15
        slider_property_number = 2
        slider_datatype = 'short'
        slider_value = 50
        self.test.setProperty(slider_node_id, slider_port, slider_wuclass_id, slider_property_number, slider_datatype, slider_value)

        binary_sensor_node_id = 2
        binary_sensor_port = 1
        binary_sensor_wuclass_id = 10
        binary_property_number = 1
        binary_datatype = 'boolean'
        binary_value = 1
        self.test.setProperty(binary_sensor_node_id, binary_sensor_port, binary_sensor_wuclass_id, binary_property_number, binary_datatype, binary_value)
        
        sound_node_id = 2
        sound_port = 3
        sound_wuclass_id = 204
        sound_property_number = 0
        	
        for loop in xrange(0,20):
                gevent.sleep(0.1)
                res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
                if res == (binary_value == 1):
                        break
        self.assertNotEqual(loop, 20)

        binary_value = 0
        self.test.setProperty(binary_sensor_node_id, binary_sensor_port, binary_sensor_wuclass_id, binary_property_number, binary_datatype, binary_value)
        for loop in xrange(0,20):
                gevent.sleep(0.1)
                res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
                if res == (binary_value == 1):
                        break
        self.assertNotEqual(loop, 20)

    def test_strength_propagate(self):        
        nodes_info = self.test.discovery()
#	self.test.setLocation(3, "/WuKong/slider")
#	self.test.setLocation(2, "/WuKong/binary")
#	self.test.setLocation(4, "/WuKong/sound")
        self.test.loadApplication("applications/three_components_in_one_device")
    	self.test.mapping(nodes_info)
    	self.test.deploy_with_discovery()
	gevent.sleep(10)

        slider_node_id = 2
        slider_port = 2
        slider_wuclass_id = 15
        slider_property_number = 2
        slider_datatype = 'short'
        slider_value = 50
        self.test.setProperty(slider_node_id, slider_port, slider_wuclass_id, slider_property_number, slider_datatype, slider_value)

        binary_sensor_node_id = 2
        binary_sensor_port = 1
        binary_sensor_wuclass_id = 10
        binary_property_number = 1
        binary_datatype = 'boolean'
        
        sound_node_id = 2
        sound_port = 3
        sound_wuclass_id = 204
        sound_property_number = 0
        
	for i in xrange(TEST_PROPAGATE_STRENGTH_NUMBER):
            ans = random.randint(0, 1)
            self.test.setProperty(binary_sensor_node_id, binary_sensor_port, binary_sensor_wuclass_id, binary_property_number, binary_datatype, ans)
            for loop in xrange(0,20):
                gevent.sleep(0.1)
                res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
                if res == (ans == 1):
                        break
            self.assertNotEqual(loop, 20)

    def test_basic_external_propagate(self):
	gevent.sleep(1)
	self.test.setLocation(2, "/WuKong/binary")
	self.test.setLocation(3, "/WuKong/slider")
	self.test.setLocation(4, "/WuKong/sound")
        print (2, self.test.getLocation(2))
        print (3, self.test.getLocation(3))
        print (4, self.test.getLocation(4))
	nodes_info = self.test.discovery()
        self.test.loadApplication("applications/three_components_in_three_devices") 
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

        binary_sensor_node_id = 2
        binary_sensor_port = 1
        binary_sensor_wuclass_id = 10
        binary_property_number = 1
        binary_datatype = 'boolean'
        
        sound_node_id = 4
        sound_port = 3
        sound_wuclass_id = 204
        sound_property_number = 0
        
	ans = 1
	self.test.setProperty(binary_sensor_node_id, binary_sensor_port, binary_sensor_wuclass_id, binary_property_number, binary_datatype, ans)

    	for loop in range(0,10):
	    gevent.sleep(0.1)
            res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
	    if res == (ans == 1):
	        break
        self.assertNotEqual(loop, 10)

	ans = 0
	self.test.setProperty(binary_sensor_node_id, binary_sensor_port, binary_sensor_wuclass_id, binary_property_number, binary_datatype, ans)
    	for loop in range(0,10):
	    gevent.sleep(0.1)
            res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
	    if res == (ans == 1):
		break
        self.assertNotEqual(loop, 10)


    def test_strength_external_propagate(self):
	self.test.setLocation(3, "/WuKong/slider")
	self.test.setLocation(2, "/WuKong/binary")
	self.test.setLocation(4, "/WuKong/sound")
        nodes_info = self.test.discovery()
        self.test.loadApplication("applications/three_components_in_three_devices") 
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

        binary_sensor_node_id = 2
        binary_sensor_port = 1
        binary_sensor_wuclass_id = 10
        binary_property_number = 1
        binary_datatype = 'boolean'
        
        sound_node_id = 4
        sound_port = 3
        sound_wuclass_id = 204
        sound_property_number = 0
        
	for i in xrange(TEST_PROPAGATE_STRENGTH_NUMBER):
            ans = 0
	    print "\033[44m setProperty to %d\033[m" % ans
            self.test.setProperty(binary_sensor_node_id, binary_sensor_port, binary_sensor_wuclass_id, binary_property_number, binary_datatype, ans)
	    for loop in range(0,TotalWaitTime):
		gevent.sleep(0.1)
            	res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
		if res == (ans == 1):
		    break
            self.assertNotEqual(loop, TotalWaitTime - 1)

	    ans = 1
            print "\033[44m setProperty to %d\033[m" % ans
            self.test.setProperty(binary_sensor_node_id, binary_sensor_port, binary_sensor_wuclass_id, binary_property_number, binary_datatype, ans)
            for loop in range(0,TotalWaitTime):
                gevent.sleep(0.1)
                res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
                if res == (ans == 1):
                    break
            self.assertNotEqual(loop, TotalWaitTime - 1)


if __name__ == '__main__':
    	TotalWaitTime = 100
	unittest.main()

