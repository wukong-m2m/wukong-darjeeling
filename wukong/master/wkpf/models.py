import sqlite3

connection = None
def global_conn():
    global connection
    if not connection:
        connection = bootstrap_database()
        connection.row_factory = sqlite3.Row
    return connection

# in wuclasses, there are some with node_id NULL, that are wuclasses from XML
# also the same with properties, node_id might be NULL
def bootstrap_database():
    print 'bootstraping database'
    global connection
    #connection = sqlite3.connect("standardlibrary.db")
    connection = sqlite3.connect(":memory:", check_same_thread = False)
    c = connection.cursor()

    # Network info
    c.execute('''CREATE TABLE IF NOT EXISTS wunodes
        (identity INTEGER PRIMARY KEY AUTOINCREMENT,
         id INTEGER not null,
         energy REAL,
         location TEXT)''')
    c.execute('''CREATE TABLE IF NOT EXISTS wuclasses
        (identity INTEGER PRIMARY KEY AUTOINCREMENT,
         wuclassdef_identity INTEGER,
         node_identity INTEGER,
         virtual BOOLEAN,
         FOREIGN KEY(wuclassdef_identity) REFERENCES wuclassdefs(identity),
         FOREIGN KEY(node_identity) REFERENCES nodes(identity))''')
    c.execute('''CREATE TABLE IF NOT EXISTS wuobjects
        (identity INTEGER PRIMARY KEY AUTOINCREMENT, 
         port_number INTEGER,
         wuclass_identity INTEGER,
         FOREIGN KEY(wuclass_identity) REFERENCES wuclasses(identity))''')
    c.execute('''CREATE TABLE IF NOT EXISTS wuproperties
        (identity INTEGER PRIMARY KEY AUTOINCREMENT,
         datatype TEXT,
         value TEXT,
         status TEXT,
         wupropertydef_identity INTEGER,
         wuobject_identity INTEGER,
         FOREIGN KEY(wupropertydef_identity) REFERENCES wupropertydefs(identity),
         FOREIGN KEY(wuobject_identity) REFERENCES wuobjects(identity))''')

    # Definitions
    c.execute('''CREATE TABLE IF NOT EXISTS wuclassdefs
        (identity INTEGER PRIMARY KEY AUTOINCREMENT,
         id INTEGER not null,
         name TEXT,
         virtual BOOLEAN,
         type TEXT)''')
    c.execute('''CREATE TABLE IF NOT EXISTS wupropertydefs
        (identity INTEGER PRIMARY KEY AUTOINCREMENT,
         number INTEGER,
         name TEXT,
         datatype_identity INTEGER,
         default_value_identity INTEGER,
         default_value TEXT,
         access TEXT,
         wuclass_id INTEGER,
         FOREIGN KEY(datatype_identity) REFERENCES wutypedefs(identity),
         FOREIGN KEY(default_value_identity) REFERENCES wuvaluedefs(identity))''')
    c.execute('''CREATE TABLE IF NOT EXISTS wutypedefs
        (identity INTEGER PRIMARY KEY AUTOINCREMENT,
         name TEXT,
         type TEXT)''')
    c.execute('''CREATE TABLE IF NOT EXISTS wuvaluedefs
        (identity INTEGER PRIMARY KEY AUTOINCREMENT,
         value TEXT,
         wutype_identity INTEGER,
         FOREIGN KEY(wutype_identity) REFERENCES wutypes(identity))''')
    connection.commit()
    return connection


class Definition:
  '''
  Definition for WuKong Profile Framework
  '''

  conn = global_conn()
  c = conn.cursor()
  tablename = ""

  def __init__(self):
    self.identity = None

  def __repr__(self):
    return '''
    %s(
      %r
    )''' % (self.__class__.__name__, self.__dict__)

  def __eq__(self, other):
    if isinstance(other, WuProperty):
      return other.identity == self.identity
    return NotImplemented

  def __ne__(self, other):
    result = self.__eq__(other)
    if result is NotImplemented:
      return result
    return not result

  @classmethod
  def all(cls):
    return cls.where()

  @classmethod
  def find(cls, **criteria):
    results = cls.where(**criteria)
    if results:
      return results[0]
    return None

  @classmethod
  def where(cls, **criteria):
    '''
    Query should return a list of matched definition objects from database
    Criteria should only contain columns in table
    '''

    criteria = map(lambda x: "%s='%s'".replace('None', 'NULL') % (x[0], x[1]), criteria.items())
    where = "WHERE " + " AND ".join(criteria) if len(criteria) > 0 else ""

    results = cls.c.execute("SELECT * from %s %s" % (cls.tablename, where)).fetchall()
    return map(lambda result: cls(*result), results)

  def save(self):
    '''
    Only saves table attributes
    '''

    columns = self.__class__.columns
    items = {}
    for x, y in self.__dict__.items():
          items.setdefault(x, y)
    t = tuple([items[key] for key in columns])
    q = "(" + ",".join(["?"] * len(t)) + ")"
    query_str = "INSERT or REPLACE into %s values %s" % (self.__class__.tablename, q)
    self.__class__.c.execute(query_str, t)
    self.__class__.conn.commit()
    self.identity = self.__class__.c.lastrowid



class WuComponent(Definition):
  def __init__(self, component_index, location, group_size, reaction_time,
          type, application_hashed_name, properties_with_default_values=[]):
    self.index = component_index
    self.location = location
    self.group_size = group_size # int
    self.reaction_time = reaction_time # float
    self.type = type # wuclass name
    self.application_hashed_name = application_hashed_name
    self.properties_with_default_values = properties_with_default_values

    self.instances = [] # WuObjects allocated on various Nodes after mapping
    self.heartbeatgroups = []

class WuLink(Definition):
  def __init__(self, from_component_index, from_property_id,
          to_component_index, to_property_id, to_wuclass_id):
    self.from_component_index = from_component_index
    self.from_property_id = from_property_id
    self.to_component_index = to_component_index
    self.to_property_id = to_property_id
    self.to_wuclass_id = to_wuclass_id

########### in db #####################


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

# Network info
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
