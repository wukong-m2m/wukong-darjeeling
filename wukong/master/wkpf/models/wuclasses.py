from base import Definition

class WuClass(Definition):
  '''
  WuClass object
  '''

  tablename = 'wuclasses'

  # Maintaining an ordered list for save function
  columns = ['identity', 'wuclassdef_identity', 'node_identity', 
      'virtual']

  @classmethod
  def create(cls, wuclassdef, node, virtual):
    wuclass = WuClass(None, wuclassdef.identity, node.identity, virtual)
    wuclass.save()
    return wuclass

  def __init__(self, identity, wuclassdef_identity, node_identity, virtual):
    self.identity = identity
    self.wuclassdef_identity = wuclassdef_identity
    self.node_identity = node_identity
    self.virtual = virtual

  def wunode(self):
    r = (self.node_identity,)
    where = "WHERE identity=?"
    result = self.__class__.c.execute("SELECT * from wunodes %s" % (where),
        r).fetchone()
    return WuNode(*list(result))

  def wuclassdef(self):
    r = (self.wuclassdef_identity,)
    where = "WHERE identity=?"
    result = self.__class__.c.execute("SELECT * from wuclassdefs %s" % (where),
        r).fetchone()
    return WuClassDef(*list(result))

