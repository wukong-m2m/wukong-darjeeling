import os, sys
import unittest
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../..'))
import test_environment_device

class TestLocation(unittest.TestCase):
    def setUp(self):
        self.dev = test_environment_device.WuTest('/dev/tty.usbserial-A9OJFT55')

    def test_basic_locationAPI(self):
        ans = 'WuKong'
        self.dev.setLocation(ans)
        location = self.dev.getLocation()
        self.assertEqual(location, ans)

    def test_strength_location(self):
        for i in xrange(5):
            ans = 'WuKong'
            self.dev.setLocation(ans)
            location = self.dev.getLocation()
            self.assertEqual(location, ans)

    def test_error_location(self):
        ans = ''
        self.dev.setLocation(ans)
        location = self.dev.getLocation()
        self.assertEqual(location, ans)

        ans = ' '
        self.dev.setLocation(ans)
        location = self.dev.getLocation()
        self.assertEqual(location, ans)

        ans = '   '
        self.dev.setLocation(ans)
        location = self.dev.getLocation()
        self.assertEqual(location, ans)

        ans = ''
        for i in xrange(50):
            ans = ans + ' '
        self.dev.setLocation(ans)
        location = self.dev.getLocation()
        self.assertEqual(location, ans)



if __name__ == '__main__':
    unittest.main()

