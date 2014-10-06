import os, sys
import time
import unittest
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../..'))
from test_environment_device import WuTest
from configuration import *


class TestProperty(unittest.TestCase):

    def setUp(self):
        self.test = WuTest(False, False)

    def test_basic_propertyAPI(self):
        node_id = 2

        port = 1
        wuclassID = 204

        property_number = 0
        ans = 0
        self.test.setProperty(node_id, port, wuclassID, property_number, 1, ans)
        res = self.test.getProperty(node_id, port, wuclassID, property_number)

        ans = 1
        self.test.setProperty(node_id, port, wuclassID, property_number, 1, ans)
        res = self.test.getProperty(node_id, port, wuclassID, property_number)
        self.assertEqual(res[0], ans)

        property_number = 1
        ans = 1
        self.test.setProperty(node_id, port, wuclassID, property_number, 0, ans)
        res = self.test.getProperty(node_id, port, wuclassID, property_number)
        self.assertEqual(res[0], ans)

        ans = 4
        self.test.setProperty(node_id, port, wuclassID, property_number, 0, ans)
        res = self.test.getProperty(node_id, port, wuclassID, property_number)
        self.assertEqual(res[0], ans)

        ans = 100
        self.test.setProperty(node_id, port, wuclassID, property_number, 0, ans)
        res = self.test.getProperty(node_id, port, wuclassID, property_number)
        self.assertEqual(res[0], ans)

if __name__ == '__main__':
    unittest.main()
