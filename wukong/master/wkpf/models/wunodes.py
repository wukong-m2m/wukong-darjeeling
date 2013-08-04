from base import Definition

class WuNode(Definition):
  '''
  Node object
  '''

  tablename = 'wunodes'

  # Maintaining an ordered list for save function
  columns = ['identity', 'id', 'energy', 'location']

  @classmethod
  def create(cls, id, location, energy=100.0):
    node = WuNode(None, id, location, energy)
    node.save()
    return node

  def __init__(self, identity, id, location, energy=100.0):
    self.identity = identity
    self.id = id
    self.location = location
    self.energy = energy

  def wuclasses(self):
    '''
    Will query from database
    '''

    wuclasses_cache = []
    r = (self.identity,)
    where = "WHERE node_identity=?"
    results = self.__class__.c.execute("SELECT * from wuclasses %s" % (where), r).fetchall()
    for result in results:
      wuclasses_cache.append(WuClass(*list(result)))
    return wuclasses_cache

  def wuobjects(self, force=False):
    '''
    Will query from database
    '''

    wuobjects_cache = []
    for wuclass_cache in self.wuclasses():
      r = (wuclass_cache.identity,)
      where = "WHERE wuclass_identity=?"
      results = self.__class__.c.execute("SELECT * from wuobjects %s" % (where), r).fetchall()
      for result in results:
        wuobjects_cache.append(WuObject(*list(result)))
    return wuobjects_cache

  def isResponding(self):
    return len(self.wuclasses()) > 0 or len(self.wuobjects()) > 0
