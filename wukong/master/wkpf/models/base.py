import sys
from blinker import signal
from cache import *
import sqlite3

connection = None
def global_conn():
    global connection
    if not connection:
        connection = bootstrap_database()
        connection.row_factory = sqlite3.Row
    return connection

def bootstrap_wupropertydefs(c):
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

def bootstrap_wutypedefs(c):
  c.execute('''CREATE TABLE IF NOT EXISTS wutypedefs
    (identity INTEGER PRIMARY KEY AUTOINCREMENT,
     name TEXT,
     type TEXT)''')

def bootstrap_wuclassdefs(c):
  c.execute('''CREATE TABLE IF NOT EXISTS wuclassdefs
    (identity INTEGER PRIMARY KEY AUTOINCREMENT,
     id INTEGER not null,
     name TEXT,
     virtual BOOLEAN,
     type TEXT)''')

def bootstrap_wuvaluedefs(c):
  c.execute('''CREATE TABLE IF NOT EXISTS wuvaluedefs
    (identity INTEGER PRIMARY KEY AUTOINCREMENT,
     value TEXT,
     wutype_identity INTEGER,
     FOREIGN KEY(wutype_identity) REFERENCES wutypes(identity))''')

def bootstrap_wuproperties(c):
  c.execute('''CREATE TABLE IF NOT EXISTS wuproperties
    (identity INTEGER PRIMARY KEY AUTOINCREMENT,
     datatype TEXT,
     value TEXT,
     status TEXT,
     wupropertydef_identity INTEGER,
     wuobject_identity INTEGER,
     FOREIGN KEY(wupropertydef_identity) REFERENCES wupropertydefs(identity),
     FOREIGN KEY(wuobject_identity) REFERENCES wuobjects(identity))''')

def bootstrap_wuclasses(c):
  c.execute('''CREATE TABLE IF NOT EXISTS wuclasses
    (identity INTEGER PRIMARY KEY AUTOINCREMENT,
     wuclassdef_identity INTEGER,
     node_identity INTEGER,
     virtual BOOLEAN,
     FOREIGN KEY(wuclassdef_identity) REFERENCES wuclassdefs(identity),
     FOREIGN KEY(node_identity) REFERENCES nodes(identity))''')

def bootstrap_wuobjects(c):
  c.execute('''CREATE TABLE IF NOT EXISTS wuobjects
    (identity INTEGER PRIMARY KEY AUTOINCREMENT, 
     port_number INTEGER,
     wuclass_identity INTEGER,
     FOREIGN KEY(wuclass_identity) REFERENCES wuclasses(identity))''')

def bootstrap_wunodes(c):
  c.execute('''CREATE TABLE IF NOT EXISTS wunodes
    (identity INTEGER PRIMARY KEY AUTOINCREMENT,
     id INTEGER not null,
     energy REAL,
     location TEXT)''')


# in wuclasses, there are some with node_id NULL, that are wuclasses from XML
# also the same with properties, node_id might be NULL
def bootstrap_database():
    print 'bootstraping database'
    global connection
    #connection = sqlite3.connect("standardlibrary.db")
    connection = sqlite3.connect(":memory:", check_same_thread = False)
    c = connection.cursor()

    # Network info
    bootstrap_wunodes(c)
    bootstrap_wuclasses(c)
    bootstrap_wuobjects(c)
    bootstrap_wuproperties(c)

    # Definitions
    bootstrap_wuclassdefs(c)
    bootstrap_wupropertydefs(c)
    bootstrap_wutypedefs(c)
    bootstrap_wuvaluedefs(c)
    connection.commit()
    return connection


cache = None
def global_cache():
    global cache
    if not cache:
        cache = KeyValueStore()
    return cache

class Definition:
  '''
  Definition for WuKong Profile Framework
  '''

  conn = global_conn()
  c = conn.cursor()
  tablename = ""
  cache = global_cache()

  def __init__(self):
    self.identity = None

  def __repr__(self):
    return '''
    %s(
      %r
    )''' % (self.__class__.__name__, self.__dict__)

  def __eq__(self, other):
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

    print "criteria is %r" % (criteria)
    print "cache content has %r for %s" % (cls.cache.get(cls.__name__), cls.__name__)

    def match_attributes(x):
      all(item in x.__dict__.items() for item in criteria.items())

    if cls.cache.get(cls.__name__):
      results = filter(match_attributes, cls.cache.get(cls.__name__))
      print "results %r" % (results)
      if results:
        return results

    print "cache miss"

    # cache miss
    query_params = map(lambda x: "%s='%s'".replace('None', 'NULL') % (x[0], x[1]), criteria.items())
    where = "WHERE " + " AND ".join(query_params) if len(query_params) > 0 else ""
    results = cls.c.execute("SELECT * from %s %s" % (cls.tablename, where)).fetchall()
    results = map(lambda result: cls(*result), results)

    cache_list = cls.cache.setdefault(cls.__name__, set())
    cache_list |= set(results)

    print "cache_list %r" % (cache_list)

    return filter(match_attributes, cls.cache.get(cls.__name__))

  def _attributes(self):
    attributes = self.__class__.columns
    items = {}
    for x, y in self.__dict__.items():
      items.setdefault(x, y)
    return items

  def save(self):
    '''
    Only saves table attributes
    '''

    t = tuple([self._attributes()[key] for key in columns])
    q = "(" + ",".join(["?"] * len(t)) + ")"
    query_str = "INSERT or REPLACE into %s values %s" % (self.__class__.tablename, q)
    self.__class__.c.execute(query_str, t)
    self.__class__.conn.commit()
    self.identity = self.__class__.c.lastrowid
