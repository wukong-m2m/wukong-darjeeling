import os, sys
import unittest
import time
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../..'))
from test_environment_device import WuTest


class TestDeploy(unittest.TestCase):
    def setUp(self):
        self.test = WuTest(False, False)

    def test_application(self):        
        self.test.setLocation(self.test.devs[0].node_id, "/WuKong/node1")
        self.test.setLocation(self.test.devs[1].node_id, "/WuKong/node2")
        self.test.setLocation(self.test.devs[2].node_id, "/WuKong/node3")
        nodes_info = self.test.discovery()
        self.test.loadApplication("applications/three_components_in_one_device") 
    	self.test.mapping(nodes_info)
    	res = self.test.deploy_with_discovery()
        self.assertTrue(res)
        time.sleep(1) # Allow the nodes some time to reboot

    def test_super_application(self):
        for i in xrange(10):
            self.test.setLocation(self.test.devs[0].node_id, "/WuKong/node1")
            self.test.setLocation(self.test.devs[1].node_id, "/WuKong/node2")
            self.test.setLocation(self.test.devs[2].node_id, "/WuKong/node3")
            nodes_info = self.test.discovery()
            self.test.loadApplication("applications/three_components_in_one_device") 
            self.test.mapping(nodes_info)
            res = self.test.deploy_with_discovery()
            self.assertTrue(res)
            time.sleep(1) # Allow the nodes some time to reboot

if __name__ == '__main__':
    unittest.main()

