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
        nodes_info = self.test.discovery()
        self.test.loadApplication("applications/9d07e0e08af7f96cf45be0112b9ccfbe") 
    	self.test.mapping(nodes_info)
    	self.test.deploy_with_discovery()

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
        
        res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
        self.assertEqual(res, 1)

        binary_value = 0
        self.test.setProperty(binary_sensor_node_id, binary_sensor_port, binary_sensor_wuclass_id, binary_property_number, binary_datatype, binary_value)
        res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
        self.assertEqual(res, 0)

    def test_strength_propagate(self):        
        nodes_info = self.test.discovery()
        self.test.loadApplication("applications/9d07e0e08af7f96cf45be0112b9ccfbe") 
    	self.test.mapping(nodes_info)
    	self.test.deploy_with_discovery()

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
            res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
            self.assertEqual(res, ans)

    def test_basic_external_propagate(self):
        nodes_info = self.test.discovery()
        self.test.loadApplication("applications/1d4b34c75f6feae66e45a9504b3ab8c6") 
    	self.test.mapping(nodes_info)
    	self.test.deploy_with_discovery()

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
	time.sleep(SLEEP_SECS)
        res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
        self.assertEqual(res, ans)

	ans = 0
	self.test.setProperty(binary_sensor_node_id, binary_sensor_port, binary_sensor_wuclass_id, binary_property_number, binary_datatype, ans)
	time.sleep(SLEEP_SECS)
	res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
        self.assertEqual(res, ans)


    def test_strength_external_propagate(self):
        nodes_info = self.test.discovery()
        self.test.loadApplication("applications/1d4b34c75f6feae66e45a9504b3ab8c6") 
    	self.test.mapping(nodes_info)
    	self.test.deploy_with_discovery()

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
            ans = random.randint(0, 1)
            self.test.setProperty(binary_sensor_node_id, binary_sensor_port, binary_sensor_wuclass_id, binary_property_number, binary_datatype, ans)
	    time.sleep(SLEEP_SECS)
            res = self.test.getProperty(sound_node_id, sound_port, sound_wuclass_id, sound_property_number)
            self.assertEqual(res, ans)


if __name__ == '__main__':
    unittest.main()

