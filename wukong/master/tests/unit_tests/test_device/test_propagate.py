import os, sys
import unittest
import random
import time
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../..'))
from test_environment_device import WuTest
from configuration import *

class TestPropagate(unittest.TestCase):
    def setUp(self):
        self.test = WuTest(False, False)

    def test_basic_propagate(self):
        self.test.setLocation(self.test.devs[0].node_id, "/WuKong/node1")
        self.test.setLocation(self.test.devs[1].node_id, "/WuKong/node2")
        self.test.setLocation(self.test.devs[2].node_id, "/WuKong/node3")

        nodes_info = self.test.discovery()
        self.test.loadApplication("applications/three_components_in_one_device") 
    	self.test.mapping(nodes_info)
    	self.test.deploy_with_discovery()

        time.sleep(1)

        binary_testsensor_node_id = self.test.devs[0].node_id
        binary_testsensor_port = 1
        binary_testsensor_wuclass_id = 1004
        binary_testsensor_property_number = 0

        integer_testsensor_node_id = self.test.devs[0].node_id
        integer_testsensor_port = 2
        integer_testsensor_wuclass_id = 1006
        integer_testsensor_property_number = 0

        sound_node_id = self.test.devs[0].node_id
        sound_port = 3
        sound_wuclass_id = 2005
        sound_onoff_property_number = 0
        sound_freq_property_number = 1

        # Test on/off
        self.test.setProperty(binary_testsensor_node_id, binary_testsensor_port, binary_testsensor_wuclass_id, binary_testsensor_property_number, 'boolean', 0)
        res = self.test.getProperty(binary_testsensor_node_id, binary_testsensor_port, binary_testsensor_wuclass_id, binary_testsensor_property_number)
        self.assertEqual(res, 0)
        res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_onoff_property_number)
        self.assertEqual(res, 0)

        self.test.setProperty(binary_testsensor_node_id, binary_testsensor_port, binary_testsensor_wuclass_id, binary_testsensor_property_number, 'boolean', 1)
        res = self.test.getProperty(binary_testsensor_node_id, binary_testsensor_port, binary_testsensor_wuclass_id, binary_testsensor_property_number)
        self.assertEqual(res, 1)
        res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_onoff_property_number)
        self.assertEqual(res, 1)


        # Test freq
        self.test.setProperty(integer_testsensor_node_id, integer_testsensor_port, integer_testsensor_wuclass_id, integer_testsensor_property_number, 'short', 50)
        res = self.test.getProperty(integer_testsensor_node_id, integer_testsensor_port, integer_testsensor_wuclass_id, integer_testsensor_property_number)
        self.assertEqual(res, 50)
        res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_freq_property_number)
        self.assertEqual(res, 50)

        self.test.setProperty(integer_testsensor_node_id, integer_testsensor_port, integer_testsensor_wuclass_id, integer_testsensor_property_number, 'short', 123)
        res = self.test.getProperty(integer_testsensor_node_id, integer_testsensor_port, integer_testsensor_wuclass_id, integer_testsensor_property_number)
        self.assertEqual(res, 123)
        res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_freq_property_number)
        self.assertEqual(res, 123)

 #    def test_strength_propagate(self):        
 #        nodes_info = self.test.discovery()
 #        self.test.loadApplication("applications/three_components_in_one_device") 
 #    	self.test.mapping(nodes_info)
 #    	self.test.deploy_with_discovery()
 #        time.sleep(10)       
 
 #        binary_testsensor_node_id = self.test.devs[0].node_id
 #        binary_testsensor_port = 1
 #        binary_testsensor_wuclass_id = 1004
 #        binary_property_number = 0
 #        binary_datatype = 'boolean'

 #        integer_testsensor_node_id = self.test.devs[0].node_id
 #        integer_testsensor_port = 2
 #        integer_testsensor_wuclass_id = 1006
 #        integer_testsensor_property_number = 0
 #        integer_testsensor_datatype = 'short'
 #        integer_testsensor_value = 50
 #        self.test.setProperty(integer_testsensor_node_id, integer_testsensor_port, integer_testsensor_wuclass_id, integer_testsensor_property_number, integer_testsensor_datatype, integer_testsensor_value)
        
 #        sound_node_id = self.test.devs[0].node_id
 #        sound_port = 3
 #        sound_wuclass_id = 2005
 #        sound_property_number = 0
        
	# for i in xrange(TEST_PROPAGATE_STRENGTH_NUMBER):
 #            ans = random.randint(0, 1)
 #            self.test.setProperty(binary_testsensor_node_id, binary_testsensor_port, binary_testsensor_wuclass_id, binary_property_number, binary_datatype, ans)
 #            res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
 #            self.assertEqual(res, ans)

 #    def test_basic_external_propagate(self):
 #        time.sleep(1)
 #        self.test.setLocation(self.test.devs[0].node_id, "/WuKong/binary")
 #        self.test.setLocation(self.test.devs[1].node_id, "/WuKong/slider")
 #        self.test.setLocation(self.test.devs[2].node_id, "/WuKong/sound")
 #        print (self.test.devs[0], self.test.getLocation(self.test.devs[0]))
 #        print (self.test.devs[1], self.test.getLocation(self.test.devs[1]))
 #        print (self.test.devs[2], self.test.getLocation(self.test.devs[2]))
 #        nodes_info = self.test.discovery()
 #        self.test.loadApplication("applications/three_components_in_three_devices") 
 #    	self.test.mapping(nodes_info)
 #    	self.test.deploy_with_discovery()
 #        time.sleep(10)

 #        binary_testsensor_node_id = self.test.devs[0].node_id
 #        binary_testsensor_port = 1
 #        binary_testsensor_wuclass_id = 1004
 #        binary_property_number = 0
 #        binary_datatype = 'boolean'
        
 #        integer_testsensor_node_id = self.test.devs[1].node_id
 #        integer_testsensor_port = 2
 #        integer_testsensor_wuclass_id = 1006
 #        integer_testsensor_property_number = 0
 #        integer_testsensor_datatype = 'short'
 #        integer_testsensor_value = 50
 #        self.test.setProperty(integer_testsensor_node_id, integer_testsensor_port, integer_testsensor_wuclass_id, integer_testsensor_property_number, integer_testsensor_datatype, integer_testsensor_value)

 #        sound_node_id = 4
 #        sound_port = 3
 #        sound_wuclass_id = 2005
 #        sound_property_number = 0
        
	# ans = 1
	# self.test.setProperty(binary_testsensor_node_id, binary_testsensor_port, binary_testsensor_wuclass_id, binary_property_number, binary_datatype, ans)
 #        for loop in range(0,10):
 #            time.sleep(0.1)
 #            res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
 #            if res == (ans == 1):
 #                break
 #        self.assertNotEqual(loop, 10)       

	# ans = 0
	# self.test.setProperty(binary_testsensor_node_id, binary_testsensor_port, binary_testsensor_wuclass_id, binary_property_number, binary_datatype, ans)
 #        for loop in range(0,10):
 #            time.sleep(0.1)
 #            res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
 #            if res == (ans == 0):
 #                break
 #        self.assertNotEqual(loop, 10)  

 #    def test_strength_external_propagate(self):
 #        self.test.setLocation(self.test.devs[0], "/WuKong/binary")
 #        self.test.setLocation(self.test.devs[1], "/WuKong/slider")
 #        self.test.setLocation(self.test.devs[2], "/WuKong/sound")
 #        nodes_info = self.test.discovery()
 #        self.test.loadApplication("applications/three_components_in_three_devices") 
 #    	self.test.mapping(nodes_info)
 #    	self.test.deploy_with_discovery()
 #        time.sleep(10) 

 #        binary_testsensor_node_id = self.test.devs[0].node_id
 #        binary_testsensor_port = 1
 #        binary_testsensor_wuclass_id = 1004
 #        binary_property_number = 0
 #        binary_datatype = 'boolean'
        
 #        integer_testsensor_node_id = self.test.devs[1].node_id
 #        integer_testsensor_port = 2
 #        integer_testsensor_wuclass_id = 1006
 #        integer_testsensor_property_number = 0
 #        integer_testsensor_datatype = 'short'
 #        integer_testsensor_value = 50
 #        self.test.setProperty(integer_testsensor_node_id, integer_testsensor_port, integer_testsensor_wuclass_id, integer_testsensor_property_number, integer_testsensor_datatype, integer_testsensor_value)

 #        sound_node_id = self.test.devs[2].node_id
 #        sound_port = 3
 #        sound_wuclass_id = 2005
 #        sound_property_number = 0
        
 #    	for i in xrange(TEST_PROPAGATE_STRENGTH_NUMBER):
 #            ans = random.randint(0, 1)
 #            print "\033[44m setProperty to %d\033[m" % ans
 #            self.test.setProperty(binary_testsensor_node_id, binary_testsensor_port, binary_testsensor_wuclass_id, binary_property_number, binary_datatype, ans)
 #            for loop in range(0,10):
 #                time.sleep(0.1)
 #                res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
 #                if res == (ans == 1):
 #                    break
 #            self.assertNotEqual(loop, 10)


if __name__ == '__main__':
    unittest.main()

