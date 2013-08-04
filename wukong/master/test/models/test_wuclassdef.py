# coding: spec

from ..test_support import *

describe 'WuClassDef':
  before_each:
    self.wuclassdef = WuClassDef.create('12', 'Test', True, '321')

  it 'could create a WuClassDef object with correct attributes':
    self.wuclassdef.id |should| equal_to('12')
    self.wuclassdef.name |should| equal_to('Test')
    self.wuclassdef.virtual |should| equal_to(True)
    self.wuclassdef.type |should| equal_to('321')
