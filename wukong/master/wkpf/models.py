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
    print 'bootstraping database "', "standardlibrary.db", '"'
    global connection
    connection = sqlite3.connect("standardlibrary.db")
    c = connection.cursor()
    c.execute('''CREATE TABLE IF NOT EXISTS nodes
        (identity INTEGER PRIMARY KEY,
         id INTEGER, 
         location TEXT)''')
    c.execute('''CREATE TABLE IF NOT EXISTS wuclasses
        (identity INTEGER PRIMARY KEY,
         id INTEGER,
         name TEXT,
         virtual BOOLEAN,
         type TEXT,
         node_id INTEGER, 
         FOREIGN KEY(node_id) REFERENCES nodes(id))''')
    c.execute('''CREATE TABLE IF NOT EXISTS properties
        (identity INTEGER PRIMARY KEY,
         id INTEGER, 
         name TEXT,
         datatype TEXT,
         access TEXT,
         value TEXT,
         status TEXT,
         wuclass_id INTEGER,
         node_id INTEGER,
         FOREIGN KEY(node_id) REFERENCES wuclasses(node_id),
         FOREIGN KEY(wuclass_id) REFERENCES wuclasses(id))''')
    c.execute('''CREATE TABLE IF NOT EXISTS wuobjects
        (identity INTEGER PRIMARY KEY, 
         node_id INTEGER, 
         port_number INTEGER,
         wuclass_id INTEGER,
         FOREIGN KEY(wuclass_id) REFERENCES wuclasses(id)
         FOREIGN KEY(node_id) REFERENCES nodes(id))''')
    c.execute('''CREATE TABLE IF NOT EXISTS components
        (identity INTEGER PRIMARY KEY,
         component_index INTEGER,
         location TEXT,
         group_size INTEGER,
         reaction_time REAL,
         type TEXT,
         application_hashed_name TEXT)''')
    c.execute('''CREATE TABLE IF NOT EXISTS links
        (id INTEGER PRIMARY KEY,
         from_component_index INTEGER,
         from_property_id INTEGER,
         to_component_index INTEGER,
         to_property_id INTEGER,
         to_wuclass_id INTEGER,
         FOREIGN KEY(to_wuclass_id) REFERENCES wuclasses(id),
         FOREIGN KEY(from_component_index) REFERENCES components(component_index),
         FOREIGN KEY(to_component_index) REFERENCES components(component_index))''')
    c.execute('''CREATE TABLE IF NOT EXISTS datatypes
        (identity INTEGER PRIMARY KEY,
         name TEXT,
         type TEXT)''')
    c.execute('''CREATE TABLE IF NOT EXISTS datatype_values
        (identity INTEGER PRIMARY KEY,
         value TEXT,
         datatype_id INTEGER,
         FOREIGN KEY(datatype_id) REFERENCES datatypes(identity))''')
    connection.commit()
    return connection

class WuComponent:
    def __init__(self, component_index, location, group_size, reaction_time,
            type, application_hashed_name, properties_with_default_values=[]):
        self.index = component_index
        self.location = location
        self.group_size = group_size # int
        self.reaction_time = reaction_time # float
        self.type = type # wuclass name
        self.application_hashed_name = application_hashed_name
        # a list of tuples with property name and default value 
        self.properties_with_default_values = properties_with_default_values
        self.instances = [] # WuObjects allocated on various Nodes after mapping
        self.heartbeatgroups = []

    def __repr__(self):
        return 'WuComponent#(\n\tcomponent_index=%d,\n\tlocationquery=%s,\n\tgroup_size=%d,\n\treaction_time=%f,\n\ttype=%s,\n\tinstances=%r,\n\tdefault_properties=%r)' % (self.index, self.location, self.group_size, self.reaction_time, self.type, self.instances, self.properties_with_default_values)

class WuLink:
    def __init__(self, from_component_index, from_property_id,
            to_component_index, to_property_id, to_wuclass_id, id=None):
        self.id = id
        self.from_component_index = from_component_index
        self.from_property_id = from_property_id
        self.to_component_index = to_component_index
        self.to_property_id = to_property_id
        self.to_wuclass_id = to_wuclass_id

########### in db #####################

class WuValue:
    def __init__(self, value, datatype_id, identity=None):
        self.value = value
        self.datatype_id = datatype_id
        self.identity = identity

    def __repr__(self):
        return "WuValue#(\n\tvalue=%s,\n\tdatatype_id=%d)" % (self.value, self.datatype_id)

    def save(self):
        c = global_conn().cursor()
        t = (self.identity, self.value, self.datatype_id)
        c.execute("insert or replace into datatype_values values (?,?,?)", t)
        global_conn().commit()
        self.identity = c.lastrowid

class WuType:
    @classmethod
    def all(cls):
        return WuType.where()

    @classmethod
    def where(cls, **criteria):
        criteria = " AND ".join(map(lambda x: "%s='%s'" % (x[0], x[1]),
                criteria.items()))
        criteria = criteria.replace('None', 'NULL')
        c = global_conn().cursor()
        types = []
        if criteria:
            types_rows = c.execute("SELECT * from datatypes WHERE %s" % (criteria)).fetchall()
        else:
            types_rows = c.execute("SELECT * from datatypes").fetchall()
        for types_row in types_rows:
            values = []
            t = (types_row['identity'],)
            values_rows = c.execute("SELECT * from datatype_values WHERE datatype_id=?", t).fetchall()
            
            values = []
            for values_row in values_rows:
                values.append(WuValue(
                            values_row['value'],
                            values_row['datatype_id'],
                            values_row['identity']
                            ))

            types.append(WuType(
                        types_row['name'], 
                        types_row['type'], 
                        values,
                        types_row['identity']))
            
        return types

    @classmethod
    def find(cls, id):
        return WuType.where(id=id)

    @classmethod
    def findByName(cls, name):
        return WuType.where(name=name)

    def __init__(self, name, type, values, identity=None):
        self.name = name
        self.type = type
        self.values = values
        self.identity = identity

    def __repr__(self):
        return "WuType#(\n\tname=%s,\n\ttype=%s,\n\tvalues=%s)" % (self.name, self.type,
                self.values)

    def save(self):
        c = global_conn().cursor()
        t = (self.identity, self.name, self.type)
        c.execute("insert or replace into datatypes values (?,?,?)", t)
        global_conn().commit()
        self.identity = c.lastrowid
        new_values = []
        for value in self.values:
            if type(value) != WuValue:
                new_values.append(WuValue(value, self.identity))
        self.values = new_values
        map(lambda x: x.save(), self.values) # don't need to assign back

class Node:
    @classmethod
    def all(cls):
        return Node.where()

    @classmethod
    def where(cls, **criteria):
        criteria = " AND ".join(map(lambda x: "%s='%s'" % (x[0], x[1]),
                criteria.items()))
        criteria = criteria.replace('None', 'NULL')
        c = global_conn().cursor()
        nodes = []
        if criteria:
            rows = c.execute("SELECT * from nodes WHERE %s" % (criteria)).fetchall()
        else:
            rows = c.execute("SELECT * from nodes").fetchall()
        for row in rows:
            wuclasses = []
            wuobjects = []

            r = (row['id'],)
            wuclasses_rows = c.execute("SELECT * from wuclasses WHERE node_id=?",
                    r).fetchall()
            for wuclasses_row in wuclasses_rows:

                properties = []
                p = (wuclasses_row['id'],)
                properties_rows = c.execute("SELECT * from properties WHERE wuclass_id=?",
                        p).fetchall()
                for properties_row in properties_rows:
                    properties.append(WuProperty(properties_row['id'],
                                properties_row['name'],
                                properties_row['datatype'],
                                properties_row['access'],
                                properties_row['value'],
                                properties_row['status'],
                                properties_row['identity']
                                ))

                wuclasses.append(WuClass(
                            wuclasses_row['id'],
                            wuclasses_row['name'],
                            bool(wuclasses_row['virtual']),
                            wuclasses_row['type'],
                            properties,
                            wuclasses_row['node_id'],
                            wuclasses_row['identity']))

            t = (row['id'],)
            wuobjects_rows = c.execute("SELECT * from wuobjects WHERE node_id=?",
                    t).fetchall()
            for wuobjects_row in wuobjects_rows:
                print wuobjects_row['wuclass_id']
                print [x.id for x in wuclasses]
                wuclass = filter(lambda x: x.id == int(wuobjects_row['wuclass_id']),
                        wuclasses)
                # WuObjects could be virtual, where the host doesn't have WuClass
                wuclass = wuclass[0] if wuclass else None
                wuobjects.append(WuObject(
                            wuobjects_row['node_id'], 
                            wuobjects_row['port_number'],
                            wuclass,
                            wuobjects_row['identity']))

            nodes.append(Node(
                        row['id'], 
                        row['location'], 
                        wuclasses, 
                        wuobjects,
                        row['identity']))
            
        return nodes

    @classmethod
    def find(cls, id):
        return Node.where(id=id)

    def __init__(self, id, location, wuclasses=[], wuobjects=[], identity=None):
        self.id = id
        self.location = location
        self.wuclasses = wuclasses
        self.wuobjects = wuobjects
        self.identity = identity

    def __repr__(self):
        return 'Node#(\n\tid="%d",\n\tlocation="%s",\n\twuclasses=%r,\n\twuobjects=%r)' % (self.id, self.location, self.wuclasses, self.wuobjects)

    def __eq__(self, other):
        if isinstance(other, Node):
            return other.identity == self.identity
        return NotImplemented

    def __ne__(self, other):
        result = self.__eq__(other)
        if result is NotImplemented:
            return result
        return not result

    def isResponding(self):
        return self.wuclasses or self.wuobjects

    def save(self):
        c = global_conn().cursor()
        t = (self.identity, self.id, self.location,)
        c.execute("INSERT OR REPLACE INTO nodes VALUES (?,?,?)", t)
        global_conn().commit()
        self.identity = c.lastrowid
        map(lambda x: x.save(), self.wuclasses)
        map(lambda x: x.save(), self.wuobjects)

class WuClass:
    @classmethod
    def all(cls):
        return WuClass.where()

    @classmethod
    def where(cls, **criteria):
        criteria = " AND ".join(map(lambda x: "%s='%s'" % (x[0], x[1]), criteria.items()))
        criteria = criteria.replace('None', 'NULL')
        c = global_conn().cursor()
        wuclasses_rows = None
        if criteria:
            wuclasses_rows = c.execute("SELECT * from wuclasses WHERE %s" % (criteria)).fetchall()
        else:
            wuclasses_rows = c.execute("SELECT * from wuclasses").fetchall()
        wuclasses = []
        for wuclasses_row in wuclasses_rows:

            properties = []
            p = (wuclasses_row['id'],)
            properties_rows = c.execute("SELECT * from properties WHERE wuclass_id=?", p).fetchall()
            for properties_row in properties_rows:
                properties.append(WuProperty(
                            properties_row['id'],
                            properties_row['name'],
                            properties_row['datatype'],
                            properties_row['access'],
                            properties_row['value'],
                            properties_row['status'],
                            properties_row['identity']))

            wuclasses.append(WuClass(
                        wuclasses_row['id'],
                        wuclasses_row['name'],
                        bool(wuclasses_row['virtual']),
                        wuclasses_row['type'],
                        properties,
                        wuclasses_row['node_id'],
                        wuclasses_row['identity']))
        return wuclasses

    @classmethod
    def find(cls, id):
        return WuClass.where(id=id)

    @classmethod
    def findByName(cls, name):
        return WuClass.where(name=name)

    def __init__(self, id, name, virtual, type, properties=[], node_id=None, identity=None):
        self.id = id
        self.name = name
        self.virtual = virtual
        self.type = type
        self.node_id = node_id
        self.properties = properties
        def func(x):
          x.wuclass = self
          return x
        self.properties = map(func, self.properties)
        self.identity = identity

    def __repr__(self):
        return 'WuClass#(\n\tid="%d",\n\tname="%s",\n\tnode_id="%s",\n\tvirtual=%r,\n\ttype="%s",\n\tproperties=%s)' % (self.id, self.name, self.node_id, self.virtual, self.type, self.properties)

    def __eq__(self, other):
        if isinstance(other, WuClass):
            return other.identity == self.identity
        return NotImplemented

    def __ne__(self, other):
        result = self.__eq__(other)
        if result is NotImplemented:
            return result
        return not result

    def save(self):
        c = global_conn().cursor()
        t = (self.identity, self.id, self.name, self.virtual, self.type, self.node_id,)
        c.execute("INSERT OR REPLACE INTO wuclasses VALUES (?,?,?,?,?,?)", t)
        self.identity = c.lastrowid
        map(lambda x: x.save(), self.properties)

class WuProperty:
    @classmethod
    def all(cls):
        return WuProperty.where()

    @classmethod
    def where(cls, **criteria):
        criteria = " AND ".join(map(lambda x: "%s='%s'" % (x[0], x[1]), criteria.items()))
        criteria = criteria.replace('None', 'NULL')
        c = global_conn().cursor()
        if criteria:
            properties_rows = c.execute("SELECT * from properties WHERE %s" % (criteria)).fetchall()
        else:
            properties_rows = c.execute("SELECT * from properties").fetchall()
        properties = []
        for properties_row in properties_rows:
            properties.append(WuProperty(
                        properties_row['id'],
                        properties_row['name'],
                        properties_row['datatype'],
                        properties_row['access'],
                        properties_row['value'],
                        properties_row['status'],
                        properties_row['identity']))
        return properties

    @classmethod
    def find(cls, id):
        return WuProperty.where(id=id)

    def __init__(self, id, name, datatype, access, value, status=None, identity=None):
        self.id = id
        self.name = name
        self.datatype = datatype
        self.access = access
        self.value = value # default or current are the same variable
        self.status = status
        self.wuclass = None
        self.identity = None

    def __repr__(self):
        return 'WuProperty#(\n\tid="%d",\n\tname="%s",\n\tdatatype=%s,\n\taccess="%s",\n\tvalue=%s,\n\twuclass_id=%s,\n\tnode_id=%s)' % (self.id, self.name, self.datatype, self.access, self.value, self.wuclass.id, self.wuclass.node_id)

    def __eq__(self, other):
        if isinstance(other, WuProperty):
            return other.identity == self.identity
        return NotImplemented

    def __ne__(self, other):
        result = self.__eq__(other)
        if result is NotImplemented:
            return result
        return not result

    def save(self):
        c = global_conn().cursor()
        t = (self.identity, self.id, self.name, self.datatype, self.access, self.value, self.status, self.wuclass.id, self.wuclass.node_id,)
        c.execute("INSERT OR REPLACE INTO properties VALUES (?,?,?,?,?,?,?,?,?)", t)
        global_conn().commit()
        self.identity = c.lastrowid

class WuObject:
    @classmethod
    def all(cls):
        return WuObject.where()

    @classmethod
    def where(cls, **criteria):
        criteria = " AND ".join(map(lambda x: "%s='%s'" % (x[0], x[1]), criteria.items()))
        criteria = criteria.replace('None', 'NULL')
        c = global_conn().cursor()
        if criteria:
            wuobjects_rows = c.execute("SELECT * from wuobjects WHERE %s" % (criteria)).fetchall()
        else:
            wuobjects_rows = c.execute("SELECT * from wuobjects").fetchall()

        wuobjects = []
        for wuobjects_row in wuobjects_rows:
            wuclasses = WuClass.all()
            wuclass = filter(lambda x: x.id == wuobjects_row['wuclass_id'],
                    wuclasses)
            # WuObjects could be virtual, where the host doesn't have WuClass
            wuclass = wuclass[0] if wuclass else None
            wuobjects.append(WuObject(
                        wuobjects_row['node_id'], 
                        wuobjects_row['port_number'],
                        wuclass,
                        wuobjects_row['identity']))
        return wuobjects

    @classmethod
    def find(cls, id):
        return WuObject.where(id=id)

    def __init__(self, node_id, port_number, wuclass, identity=None,
            properties_with_default_values=[]):
        self.node_id = node_id
        self.port_number = port_number
        self.wuclass = wuclass # if None, means the host doesn't have WuClass, it's virtual
        self.identity = identity
        # a list of tuples with property name and default value 
        self.properties_with_default_values = properties_with_default_values
        self.hasLocalNativeWuClass = False

    def __repr__(self):
        return 'WuObject#(\n\tnode_id="%s",\n\tport_number=%d,\n\twuclass="%r",\n\tdefault_properties=%r)' % (self.node_id, self.port_number, self.wuclass, self.properties_with_default_values)

    def __eq__(self, other):
        if isinstance(other, WuObject):
            return other.identity == self.identity
        return NotImplemented

    def __ne__(self, other):
        result = self.__eq__(other)
        if result is NotImplemented:
            return result
        return not result

    def save(self):
        c = global_conn().cursor()
        if self.wuclass:
            t = (self.identity, self.node_id, self.port_number, self.wuclass.id,)
            c.execute("INSERT OR REPLACE INTO wuobjects VALUES (?,?,?,?)", t)
        else: # virtual
            t = (self.identity, self.node_id, self.port_number, None,)
            c.execute("INSERT OR REPLACE INTO wuobjects VALUES (?,?,?,?)", t)
        global_conn().commit()
        self.identity = c.lastrowid
