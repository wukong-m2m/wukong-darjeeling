import os, sys
import unittest
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../..'))
from test_environment_device import WuTest


class TestDeploy(unittest.TestCase):
    def setUp(self):
        self.test = WuTest(False, False)

    def test_application(self):        
        nodes_info = self.test.discovery()
        self.test.loadApplication("applications/three_components_in_one_device") 
    	self.test.mapping(nodes_info)
    	res = self.test.deploy_with_discovery()
        self.assertTrue(res)

    def test_super_application(self):
        for i in xrange(10):
            nodes_info = self.test.discovery()
            self.test.loadApplication("applications/three_components_in_one_device") 
            self.test.mapping(nodes_info)
            res = self.test.deploy_with_discovery()
            self.assertTrue(res)

if __name__ == '__main__':
    unittest.main()

