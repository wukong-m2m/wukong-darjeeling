import os, sys
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../..'))
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import unittest
#import test_environment # this test is an exception, since test_environment
#also uses parser
from wkpf.parser import *

if os.path.exists('standardlibrary.db'):
  os.remove('standardlibrary.db') 

class TestParser(unittest.TestCase):
  def test_parseLibraryXMLString(self):
    Parser.parseLibraryXMLString("""
    <WuKong>
        <WuTypedef name="MathOperator" type="enum">
            <enum value="MAX"/>
            <enum value="MIN"/>
            <enum value="AVG"/>
            <enum value="ADD"/>
            <enum value="SUB"/>
            <enum value="MULTIPLY"/>
            <enum value="DIVIDE"/>
        </WuTypedef>
    </WuKong>
    """)

    self.assertEqual(len(WuType.all()), 1)

  def test_parseLibraryXMLString2(self):
    Parser.parseLibraryXMLString("""
    <WuKong>
        <WuClass name="Generic" id="0" virtual="false" type="hard">
            <property name="dummy" datatype="short" access="readonly" />
        </WuClass>
    </WuKong>
    """)

    self.assertEqual(len(WuClass.all()), 1)
    self.assertEqual(len(WuClass.where(id="0")), 1)

if __name__ == '__main__':
  unittest.main()
