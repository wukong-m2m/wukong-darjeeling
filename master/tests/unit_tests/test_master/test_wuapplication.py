import os, sys
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../..'))
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import unittest
import test_environment
from wkpf.wuapplication import *

xml = '''
<application name="123">
<page title="_first_">
    <component type="Threshold" instanceId="19" x="343" y="206" w="100" h="100">
        <link fromProperty="output" toInstanceId="22" toProperty="on_off"/>
        <location requirement="WuKong" />
        <group_size requirement="1" />
        <reaction_time requirement="2" />
        <actionProperty  />
        <signalProperty  />
    </component>
    <component type="Numeric_Controller" instanceId="20" x="80" y="237" w="100" h="100">
        <link fromProperty="output" toInstanceId="19" toProperty="threshold"/>
        <location requirement="WuKong" />
        <group_size requirement="1" />
        <reaction_time requirement="2" />
        <actionProperty output=""  />
        <signalProperty output=""  />
    </component>
    <component type="Light_Sensor" instanceId="21" x="76" y="397" w="100" h="100">
        <link fromProperty="current_value" toInstanceId="19" toProperty="value"/>
        <location requirement="WuKong" />
        <group_size requirement="2" />
        <reaction_time requirement="2" />
        <actionProperty refresh_rate=""  />
        <signalProperty current_value="" refresh_rate=""  />
    </component>
    <component type="Light_Actuator" instanceId="22" x="633" y="224" w="100" h="100">
        <location requirement="WuKong" />
        <group_size requirement="2" />
        <reaction_time requirement="2" />
        <actionProperty on_off=""  />
        <signalProperty on_off=""  />
    </component>
</page>
</application>
'''

class TestWuApplication(unittest.TestCase):
  def setUp(self):
    self.application = WuApplication('455', '123')
    self.application.setFlowDom(parseString(xml))
    self.application.parseApplication()

  def test_parseApplication(self):
    self.assertEqual(self.application.name, '123')
    self.assertEqual(len(self.application.changesets.components), 4)
    self.assertTrue((u'output', u'gg') in self.application.changesets.components[1].properties_with_default_values)
    self.assertTrue((u'refresh_rate', u'323') in self.application.changesets.components[2].properties_with_default_values)
    self.assertTrue((u'on_off', u'false') in self.application.changesets.components[3].properties_with_default_values)

  def test_generateJava(self):
    self.application.generateJava()

    self.assertEqual(self.application.name, '123')

if __name__ == '__main__':
  unittest.main()
