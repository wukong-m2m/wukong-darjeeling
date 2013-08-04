from base import Definition

class WuObject(Definition):
  '''
  WuObject object
  '''

  tablename = 'wuobjects'

  # Maintaining an ordered list for save function
  columns = ['identity', 'port_number', 'wuclass_identity']

  @classmethod
  def create(cls, port_number, wuclass):
    wuobject = WuObject(None, port_number, wuclass.identity)
    wuobject.save()

    # Might have to make an exception here, since Property is ususally
    # not generated explicitly as it is assumed to come with WuObjects
    for wupropertydef in wuclass.wuclassdef().wupropertydefs():
      wutype = wupropertydef.wutype()
      wuvalue = wupropertydef.default_wuvalue()
      if isinstance(wuvalue, WuValueDef):
        wuvalue = wuvalue.value
      status = 0x10 #TODO: Dummy value. Niels: Don't have it in python yet
      wuproperty = WuProperty.create(wutype.name,
          wuvalue, status, wupropertydef.identity,
          wuobject.identity)

    return wuobject

  def __init__(self, identity, port_number, wuclass_identity):
    self.identity = identity
    self.port_number = port_number
    self.wuclass_identity = wuclass_identity

  def __repr__(self):
    return '''
    %s(
      %r
    )''' % (self.__class__.__name__, self.__dict__.items() + [('node_id', self.wunode().id)])

  def wunode(self):
    wuclass = self.wuclass()
    if not wuclass:
      raise Exception('WuObject has no WuClass')
    r = (wuclass.node_identity,)
    where = "WHERE identity=?"
    result = self.__class__.c.execute("SELECT * from wunodes %s" % (where),
        r).fetchone()
    return WuNode(*list(result))

  def wuclass(self):
    r = (self.wuclass_identity,)
    where = "WHERE identity=?"
    result = self.__class__.c.execute("SELECT * from wuclasses %s" % (where),
        r).fetchone()
    return WuClass(*list(result))

  def wuproperties(self):
    properties = []
    r = (self.identity,)
    where = "WHERE wuobject_identity=?"
    results = self.__class__.c.execute("SELECT * from wuproperties %s" % (where), r).fetchall()
    for result in results:
      properties.append(WuProperty(*list(result)))
    return properties
