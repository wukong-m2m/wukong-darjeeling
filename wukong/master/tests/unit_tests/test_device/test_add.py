import os, sys
import unittest
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../..'))
from test_environment_device import WuTest
from configuration import *

class TestDevicemanager(unittest.TestCase):

    def setUp(self):
    	self.test = WuTest(False, False)

    def _test_basic_add(self):
	self.test.stop()
	self.test.wait('done')
	self.test.add()
	self.test.wait('ready')
	self.test.waitDeviceReady(self.test.devs[0])
	self.test.deviceLearn(self.test.devs[0])
	r = self.test.wait('found',20)
	self.assertEqual(r, True)

    def _test_strength_add(self):
        for i in xrange(TEST_BASIC_STRENGTH_NUMBER):
		self.test.stop()
		self.test.wait('done')
		self.test.add()
		self.test.wait('ready')
		self.test.waitDeviceReady(self.test.devs[0])
		self.test.deviceLearn(self.test.devs[0])
		r = self.test.wait('found',20)
		self.assertEqual(r, True)
    def test_basic_del(self):
	self.test.stop()
	self.test.wait('done')
	self.test.add()
	self.test.wait('ready')
	self.test.deviceLearn(self.test.devs[0])
	r = self.test.wait('found',20)
	self.assertEqual(r, True)
	self.test.stop()
	self.test.wait('done')
	self.test.delete()
	self.test.wait('ready')
	self.test.deviceLearn(self.test.devs[0])
	r = self.test.wait('found',20)
	self.assertEqual(r, True)

    def test_strength_del(self):
        for i in xrange(TEST_BASIC_STRENGTH_NUMBER):
		self.test.stop()
		self.test.wait('done')
		self.test.add()
		self.test.wait('ready')
		self.test.waitDeviceReady(self.test.devs[0])
		self.test.deviceLearn(self.test.devs[0])
		r = self.test.wait('found',20)
		self.assertEqual(r, True)
		self.test.stop()
		self.test.wait('done')
		self.test.delete()
		self.test.wait('ready')
		self.test.deviceLearn(self.test.devs[0])
		r = self.test.wait('found',20)
		self.assertEqual(r, True)

if __name__ == '__main__':
    unittest.main()
