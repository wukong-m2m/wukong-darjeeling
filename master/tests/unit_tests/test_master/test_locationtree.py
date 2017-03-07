import os, sys
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../..'))
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import unittest
import test_environment

class TestLocationTree(unittest.TestCase):
  def test_function(self):
    pass

if __name__ == '__main__':
  unittest.main()
