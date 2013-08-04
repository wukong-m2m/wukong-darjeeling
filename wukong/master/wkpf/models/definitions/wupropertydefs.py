from ..base import Definition
from wutypedefs import WuTypeDef
from wuvaluedefs import WuValueDef

class WuPropertyDef(Definition):
  '''
  WuProperty Definition
  '''

  tablename = 'wupropertydefs'

  # Maintaining an ordered list for save function
  columns = ['identity', 'number', 'name', 'datatype_identity', 
      'default_value_identity', 'default_value', 'access', 'wuclass_id']

  @classmethod
  def create(cls, number, name, datatype, default, access, wuclassdef):
    '''
    datatype and default will be string
    '''
    wutype = WuTypeDef.find(name=datatype)
    if not wutype:
      raise Exception('type not found %s' % (datatype))

    wuvalue_identity = None
    if not datatype in ['short', 'boolean', 'refresh_rate']:
      wuvalue = WuValueDef.find(value=default)
      if not wuvalue:
        raise Exception('default value not found %s' % (default))
      else:
        wuvalue_identity = wuvalue.identity

    wutype = WuPropertyDef(None, number, name, wutype.identity,
        wuvalue_identity, default, access, wuclassdef.id)
    wutype.save()
    return wutype

  def __init__(self, identity, number, name, datatype_identity,
      default_value_identity, default, access, wuclass_id):
    self.identity = identity
    self.number = number
    self.name = name
    self.datatype_identity = datatype_identity
    self.default_value_identity = default_value_identity
    self.default_value = default
    self.access = access
    self.wuclass_id = wuclass_id

  def wutype(self):
    r = (self.datatype_identity,)
    where = "WHERE identity=?"
    result = self.__class__.c.execute("SELECT * from wutypedefs %s" % (where),
        r).fetchone()
    return WuTypeDef(*list(result))

  def wuclassdef(self):
    r = (self.wuclass_id,)
    where = "WHERE id=?"
    result = self.__class__.c.execute("SELECT * from wuclassdefs %s" % (where),
        r).fetchone()
    return WuClassDef(*list(result))

  def default_wuvalue(self):
    # For basic types
    if not self.default_value_identity:
      return self.default_value

    # Otherwise...
    r = (self.default_value_identity,)
    where = "WHERE identity=?"
    result = self.__class__.c.execute("SELECT * from wuvaluedefs %s" % (where),
        r).fetchone()
    return WuValueDef(*list(result))
