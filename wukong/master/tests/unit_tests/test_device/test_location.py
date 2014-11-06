import os, sys
import unittest
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../..'))
from test_environment_device import WuTest
from configuration import *

import random
import string

class TestLocation(unittest.TestCase):

    def setUp(self):
        self.test = WuTest(False, False)

    def test_basic_locationAPI(self):
        for i in xrange(len(self.test.devs)):
            node_id = self.test.devs[i].node_id

            length = random.randint(1, MAX_LOCATION_LENGTH)
            ans = "".join(random.choice(string.printable) for i in xrange(length))
            self.test.setLocation(node_id, ans)
            location = self.test.getLocation(node_id)
            self.assertEqual(location, ans)

    def test_strength_location(self):
        for i in xrange(len(self.test.devs)):
            node_id = self.test.devs[i].node_id

            for j in xrange(TEST_LOCATION_STRENGTH_NUMBER):
                ans = str(j)
                self.test.setLocation(node_id, ans)
                location = self.test.getLocation(node_id)
                self.assertEqual(location, ans)

    def test_error_location(self):
        for i in xrange(len(self.test.devs)):
            node_id = self.test.devs[i].node_id

            ans = '\x00'
            self.test.setLocation(node_id, ans)
            location = self.test.getLocation(node_id)
            self.assertEqual(location, ans)

            ans = '\xff'
            self.test.setLocation(node_id, ans)
            location = self.test.getLocation(node_id)
            self.assertEqual(location, ans)
            
            ans = '123+-*/456'
            self.test.setLocation(node_id, ans)
            location = self.test.getLocation(node_id)
            self.assertEqual(location, ans)


if __name__ == '__main__':
    unittest.main()
