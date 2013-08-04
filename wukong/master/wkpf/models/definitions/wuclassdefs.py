from ..base import Definition

class WuClassDef(Definition):
  '''
  WuClass Definition
  '''

  tablename = 'wuclassdefs'

  # Maintaining an ordered list for save function
  columns = ['identity', 'id', 'name', 'virtual', 'type']

  @classmethod
  def create(cls, id, name, virtual, type):
    wuclass = WuClassDef(None, id, name, virtual, type)
    wuclass.save()
    return wuclass

  def __init__(self, identity, id, name, virtual, type):
    self.identity = identity
    self.id = id
    self.name = name
    self.virtual = virtual
    self.type = type

  def wupropertydefs(self):
    defs = []
    r = (self.id,)
    where = "WHERE wuclass_id=?"
    results = self.__class__.c.execute("SELECT * from wupropertydefs %s" % (where), r).fetchall()
    for result in results:
      defs.append(WuPropertyDef(*list(result)))
    return defs
