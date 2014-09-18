import os, sys
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../..'))
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import unittest
from wkpf.wuapplication import *



xml = '''
<application name="123">
<page title="_first_">
    <component type="Ultrasound_Sensor" instanceId="50" x="225" y="175" w="120" h="100">
        <link fromProperty="signal" toInstanceId="52" toProperty="value"/>
        <location requirement="/WuKong/3#" />
        <group_size requirement="1" />
        <reaction_time requirement="1" />
        <signalProperty signal="" refresh_rate=""  />
    </component>
    <component type="Threshold" instanceId="52" x="446" y="177" w="120" h="100">
        <link fromProperty="output" toInstanceId="51" toProperty="on_off"/>
        <location requirement="/WuKong/3#" />
        <group_size requirement="1" />
        <reaction_time requirement="1" />
        <signalProperty operator="" threshold="" value="" output=""  />
    </component>
    <component type="Buzzer" instanceId="51" x="676" y="167" w="120" h="100">
        <location requirement="/WuKong/1#" />
        <group_size requirement="1" />
        <reaction_time requirement="1" />
        <signalProperty on_off=""  />
    </component>
</page>
</application>
'''

class TestWuAppMerger(unittest.TestCase):
  def setUp(self):
    self.application = WuApplication('455', '123')
    self.application.setFlowDom(parseString(xml))
    self.application.parseApplication()


  def test_parseApplication(self):
    self.assertEqual(self.application.name, '123')
    self.assertEqual(len(self.application.changesets.components), 3)
    self.assertTrue((u'output', u'gg') in self.application.changesets.components[1].properties_with_default_values)
    self.assertTrue((u'refresh_rate', u'323') in self.application.changesets.components[2].properties_with_default_values)
    self.assertTrue((u'on_off', u'false') in self.application.changesets.components[3].properties_with_default_values)

  # def test_generateJava(self):
  #   self.application.generateJava()

  #   self.assertEqual(self.application.name, '123')

if __name__ == '__main__':
  unittest.main()
