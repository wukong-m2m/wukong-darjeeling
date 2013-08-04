from ..base import Definition

class WuValueDef(Definition):
  '''
  WuValue Definition
  '''

  tablename = 'wuvaluedefs'

  # Maintaining an ordered list for save function
  columns = ['identity', 'value', 'wutype_identity']

  @classmethod
  def create(cls, value, wutype_identity):
    wuvalue = WuValueDef(None, value, wutype_identity)
    wuvalue.save()
    return wuvalue

  def __init__(self, identity, value, wutype_identity):
    self.identity = identity
    self.value = value
    self.wutype_identity = wutype_identity

  def wutype(self):
    r = (self.wutype_identity,)
    where = "WHERE identity=?"
    result = self.__class__.c.execute("SELECT * from wutypedefs %s" % (where),
        r).fetchone()
    return WuTypeDef(*list(result))
