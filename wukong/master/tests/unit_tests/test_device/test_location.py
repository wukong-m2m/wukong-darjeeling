import os, sys
import unittest
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../..'))
from test_environment_device import WuTest
from configuration import *

class TestLocation(unittest.TestCase):

    def setUp(self):
        self.test = WuTest()

    def test_basic_locationAPI(self):
        for i in xrange(self.test.dev_len):
            node_id = i + 2

            ans = 'WuKong'
            self.test.setLocation(node_id, ans)
            location = self.test.getLocation(node_id)
            self.assertEqual(location, ans)

    def test_strength_location(self):
        for i in xrange(self.test.dev_len):
            node_id = i + 2

            for j in xrange(5):
                ans = 'WuKong'
                self.test.setLocation(node_id, ans)
                location = self.test.getLocation(node_id)
                self.assertEqual(location, ans)

    def test_error_location(self):
        for i in xrange(self.test.dev_len):
            node_id = i + 2

            ans = ''
            self.test.setLocation(node_id, ans)
            location = self.test.getLocation(node_id)
            self.assertEqual(location, ans)

            ans = ' '
            self.test.setLocation(node_id, ans)
            location = self.test.getLocation(node_id)
            self.assertEqual(location, ans)

            ans = '\x00'
            self.test.setLocation(node_id, ans)
            location = self.test.getLocation(node_id)
            self.assertEqual(location, ans)

            ans = '\x01'
            self.test.setLocation(node_id, ans)
            location = self.test.getLocation(node_id)
            self.assertEqual(location, ans)

            ans = ''
            for j in xrange(50):
                ans = ans + ' '
            self.test.setLocation(node_id, ans)
            location = self.test.getLocation(node_id)
            self.assertEqual(location, ans)


if __name__ == '__main__':
    unittest.main()
