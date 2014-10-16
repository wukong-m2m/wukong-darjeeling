import os, sys
import unittest
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../..'))
from test_environment_device import WuTest
from configuration import *

from random import choice
from string import lowercase

class TestLocation(unittest.TestCase):

    def setUp(self):
        self.test = WuTest(False, False)

    def test_basic_locationAPI(self):
        for i in xrange(self.test.dev_len):
            node_id = self.node_ids[i]

            ans = 'WuKong'
            self.test.setLocation(node_id, ans)
            location = self.test.getLocation(node_id)
            self.assertEqual(location, ans)

    def test_strength_location(self):
        for i in xrange(self.test.dev_len):
            node_id = self.node_ids[i]

            for j in xrange(TEST_LOCATION_STRENGTH_NUMBER):
                ans = str(j)
                self.test.setLocation(node_id, ans)
                location = self.test.getLocation(node_id)
                self.assertEqual(location, ans)

    def test_error_location(self):
        for i in xrange(self.test.dev_len):
            node_id = self.node_ids[i]

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

            # random generate long location string
            ans = "".join(choice(lowercase) for i in range(TEST_LOCATION_ERROR_LENGTH))
            self.test.setLocation(node_id, ans)
            location = self.test.getLocation(node_id)
            self.assertEqual(location, ans)


if __name__ == '__main__':
    unittest.main()
