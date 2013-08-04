from ..base import Definition

class WuTypeDef(Definition):
  '''
  WuType Definition
  '''

  tablename = 'wutypedefs'

  # Maintaining an ordered list for save function
  columns = ['identity', 'name', 'type']

  @classmethod
  def create(cls, name, type):
    wutype = WuTypeDef(None, name, type)
    wutype.save()
    return wutype

  def __init__(self, identity, name, type):
      self.identity = identity
      self.name = name
      self.type = type

  def wuvalues(self):
    values = []

    if not self.type.lower() in ['short', 'boolean', 'refresh_rate']:
      r = (self.identity,)
      where = "WHERE wutype_identity=?"
      results = self.__class__.c.execute("SELECT * from wuvaluedefs %s" % (where),
          r).fetchall()

      for result in results:
        values.append(WuValueDef(*list(result)))
    return values
