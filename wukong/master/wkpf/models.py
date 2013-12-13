import sqlite3
import copy



class WuComponent:
 
  def __init__(self, component_index, location, group_size, reaction_time,
          type, application_hashed_name, properties=None):
    self.index = component_index
    self.location = location
    self.group_size = group_size # int
    self.reaction_time = reaction_time # float
    self.type = type # wuclass name
    self.application_hashed_name = application_hashed_name
    self.properties = properties  #properties without default values
    if self.properties == None:
      self.properties = []
    self.tmpid = -1     #temporary id, only used for generating wkpfdeploy.xml
    self.instances = [] # WuObjects allocated on various Nodes after mapping
    self.heartbeatgroups = []
    
class WuLink:
  def __init__(self, from_component, from_property_name,
          to_component, to_property_name):
    self.from_component = from_component
    self.from_property_name = from_property_name
    self.from_property = WuObjectFactory.wuclassdefsbyname[from_component.type].properties[from_property_name]
    self.to_component = to_component
    self.to_property_name = to_property_name
    self.to_property = WuObjectFactory.wuclassdefsbyname[to_component.type].properties[to_property_name]


########### in db #####################
class WuObjectFactory:
 
  wutypedefs = {}
  wuclassdefsbyid = {}
  wuclassdefsbyname = {}
   #property_values, a dictionary of property_name: value pairs, may not contain all properties, default will be used in that case
  @classmethod
  def createWuObject(cls, wuclassdef, wunode, port_number, virtual, property_values=None):
    wuobj = WuObject( wuclassdef, wunode, port_number, virtual, property_values)
    wunode.wuobjects[port_number] = wuobj
    return wuobj
  @classmethod
  def removeWuObject(cls,wunode, port_number):
    del wunode.wuobjects[port_number]
    return True
  
  @classmethod
  def createWuPropertyDef(cls, id, name, wutype_name, default_value,  access, wuclassdef):
    wutype = None
    if wutype_name in cls.wutypedefs.keys():
      wutype = cls.wutypedefs[wutype_name]
    if wutype == None:
      print ("wutype", wutype_name, "does not exist")
    wuprop = WuPropertyDef(id, name, wutype, default_value,  access, wuclassdef)
    wuclassdef.properties[name]=wuprop
    return wuprop
    
  @classmethod
  def createWuTypeDef(cls, name, type,values=None):
    cls.wutypedefs[name] = WuTypeDef(name,type,values)
    return cls.wutypedefs[name]
  @classmethod
  def createWuClassDef(cls, id, name, virtual, type, properties = None):
    cls.wuclassdefsbyid[id] = WuClassDef( id, name, virtual, type, properties) 
    cls.wuclassdefsbyname[name] = cls.wuclassdefsbyid[id]
    return cls.wuclassdefsbyname[name]
    
# TODO: should remove virtual from this class
class WuClassDef:
  
  # Maintaining an ordered list for save function
  #properties, a dictionary of name:wupropertydef
  def __init__(self, id, name, virtual, type, properties = None):
    self.id = id
    self.name = name
    self.virtual = virtual
    self.type = type
    self.properties = properties    #dictionary of wuproperties
    if self.properties == None:
      self.properties = {}
  
  def createWuProperty(self, name,
        default_value, wutype, access, wuclassdef):
    self.properties[name] = WuPropertyDef(id, name, default_value, wutype, access, self)
    return self.wuproperties[name]

class WuPropertyDef:

  def __init__(self,  id, name,wutype, 
      default_value,  access, wuclassdef):
    self.id = id
    self.name = name
    self.value = default_value
    self.wutype = wutype
    self.access = access
    self.wuclassdef = wuclassdef


class WuTypeDef:

  #values is a list of values
  def __init__(self, name, type,values=None):
      self.name = name
      self.wutype = type
      self.values = values
      if self.values == None:
        self.values = []
      


# Network info
class WuNode:
  
  node_dict = {}

  def __init__(self, id, location, wuclassdefs=None, wuobj=None,energy=100.0,type='wudevice'):
    self.id = id
    self.location = location
    self.wuclasses = wuclassdefs  #wuclasses[id]
    self.wuobjects = wuobj    #wuobjects[port]
    if self.wuclasses == None:
      self.wuclasses = {}
    if self.wuobjects == None:
      self.wuobjects = {}
    self.energy = energy
    self.type = type
    WuNode.node_dict[id] = self
  
  def dump(self):   
    stri = ''
    stri += "id:"+str(self.id)+'\n'
    stri += "location:"+ self.location+'\n'
    stri += "wuclasses:" +str(self.wuclasses)+'\n'
    stri += "wuobjects"+str(self.wuobjects)+'\n'
    stri += "type:"+self.type+'\n'
    return stri
    
  @classmethod
  def findById(cls, id):
    if id in cls.node_dict.keys():
      return cls.node_dict[id]
    return None
  
  @classmethod
  def saveNodes(cls, filename="../LocalData/nodes.txt"):#for debug now, will expand to support reconstructing nodes from the dump ---- Sen
      
    fin = open(filename,"w")
    for nd in WuNode.node_dict.values():
      fin.write( nd.dump())
    fin.close()
    return
  
  def isResponding(self):
    return len(self.wuclasses) > 0 or len(self.wuobjects) > 0


class WuObject:
  ZWAVE_SWITCH_PORT1 = 64
  ZWAVE_SWITCH_PORT2 = 65
  ZWAVE_SWITCH_PORT3 = 66
  ZWAVE_SWITCH_PORT4 = 67
  ZWAVE_DIMMER_PORT1 = 70
  ZWAVE_DIMMER_PORT2 = 71
  ZWAVE_DIMMER_PORT3 = 72
  ZWAVE_DIMMER_PORT4 = 73
  ZWAVE_CURTAIN_PORT1 = 73
  ZWAVE_CURTAIN_PORT2 = 74
  ZWAVE_CURTAIN_PORT3 = 75
  ZWAVE_CURTAIN_PORT4 = 76
  def __init__(self, wuclassdef, wunode, port_number, virtual, property_values = None):
    self.port_number = port_number
    self.wuclassdef = wuclassdef
    self.wunode = wunode
    self.virtual = virtual
    self.properties = wuclassdef.properties
    if property_values == None:
      property_values = {}
    for prop_name, prop_value in property_values:
      wuprop = self.wuclassdef.wuproperties[prop_name]
      new_wuprop = copy.deepcopy(wuprop)
      new_wuprop.value = prop_value
      self.properties[prop_name] = new_wuprop
      
  
  def getPropertyByName(self, name):
    return property_values[name]
