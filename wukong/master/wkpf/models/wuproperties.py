from base import Definition

class WuProperty(Definition):
  '''
  WuProperty object
  '''

  tablename = 'wuproperties'

  # Maintaining an ordered list for save function
  columns = ['identity', 'datatype', 'value', 'status', 
      'wupropertydef_identity', 'wuobject_identity']

  @classmethod
  def create(cls, datatype, value, status, wupropertydef_identity,
      wuobject_identity):
    wuproperty = WuProperty(None, datatype, value, status,
        wupropertydef_identity, wuobject_identity)
    wuproperty.save()
    return wuproperty

  def __init__(self, identity, datatype, value, status,
      wupropertydef_identity, wuobject_identity):
    self.identity = identity
    self.datatype = datatype
    self.value = value # TODO: type needs to be converted (e.g. Boolean, Short)
    self.status = status
    self.wupropertydef_identity = wupropertydef_identity
    self.wuobject_identity = wuobject_identity

  def wuobject(self):
    r = (self.wuobject_identity,)
    where = "WHERE identity=?"
    result = self.__class__.c.execute("SELECT * from wuobjects %s" % (where),
        r).fetchone()
    return WuObject(*list(result))

  def wupropertydef(self):
    r = (self.wupropertydef_identity,)
    where = "WHERE identity=?"
    result = self.__class__.c.execute("SELECT * from wupropertydefs %s" % (where),
        r).fetchone()
    return WuPropertyDef(*list(result))
