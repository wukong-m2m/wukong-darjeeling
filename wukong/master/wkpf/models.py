import sqlite3
import copy
from xml.etree import ElementTree
import xml.dom.minidom
import traceback



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
    self.deployid = -1     #temporary id, only used for generating wkpfdeploy.xml
    self.instances = [] # WuObjects allocated on various Nodes after mapping
    self.message = ""
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
  locations = {}

  def __init__(self, id, location, wuclassdefs=None, wuobj=None,energy=100.0,type='wudevice'):
    self.id = id
    if location == None:
	try:
	    self.location = WuNode.locations[id]
	except:
	    self.location = 'WuKong'
    else:	    
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
  def dumpXML(cls):   
    root = ElementTree.Element('Nodes')
    for id, node in cls.node_dict.items():
        node_element = ElementTree.SubElement(root, 'Node')
        node_element.attrib['id'] = str(id)
        node_element.attrib['type'] = str(node.type)
        location_element = ElementTree.SubElement(node_element, "Location")
        location_element.attrib['length'] = str(len(node.location))
        location_element.attrib['content'] = str(node.location)
        
        wuclasslist_element = ElementTree.SubElement(node_element,"WuClassList")
        wuclasslist_element.attrib['length'] = str(len(node.wuclasses))
        for wuclassid, wuclass in  node.wuclasses.items():
            wuclass_element = ElementTree.SubElement(wuclasslist_element,"WuCLass")
            wuclass_element.attrib['id'] = str(wuclassid)
            wuclass_element.attrib['virtual'] = str(wuclass.virtual)
        
        wuobjectlist_element = ElementTree.SubElement(node_element,"WuObjectList")
        for port, wuobject in  node.wuobjects.items():
            wuobject_element = ElementTree.SubElement(wuobjectlist_element,"WuObject")
            wuobject_element.attrib['id'] = str(wuobject.wuclassdef.id)
            wuobject_element.attrib['virtual'] = str(wuobject.virtual)
            wuobject_element.attrib['port'] = str(port)
  #          for prop_name, prop_value in wuobject.properties.items():
   #           wuproperty_element = ElementTree.SubElement(wuobject_element,"WuProperty")
    #          wuproperty_element.attrib['name'] = str(prop_name)
     #         wuproperty_element.attrib['value'] = str(prop_value)

    #for more human readable xml
    rough_stri = ElementTree.tostring(root, 'utf-8')
    xml_content = xml.dom.minidom.parseString(rough_stri)
    pretty_stri = xml_content.toprettyxml()
    return pretty_stri
  @classmethod
  def findById(cls, id):
    if id in cls.node_dict.keys():
      return cls.node_dict[id]
    return None
  
  @classmethod
  def saveNodes(cls, filename="../LocalData/nodes.xml"):#for debug now, will expand to support reconstructing nodes from the dump ---- Sen
      
    fin = open(filename,"w")
    fin.write( WuNode.dumpXML())
    fin.close()
    WuNode.locations={}
    for id in WuNode.node_dict:
      WuNode.locations[id] = WuNode.node_dict[id].location
    return
  @classmethod
  def clearNodes(cls, filename="../LocalData/nodes.xml"):
    cls.node_dict = {}
    fin = open(filename,"w")
    fin.write("")
    fin.close()
    return
  @classmethod
  def loadNodes(cls, filename="../LocalData/nodes.xml"):#for debug now, will expand to support reconstructing nodes from the dump ---- Sen
      print ('[loadNodes in models] Loading node from file', filename)
      try:
          fin = open(filename,"r")
          nodedom = xml.dom.minidom.parse(filename)
      except Exception:
          print (filename,'does not exist, initial list is empty!')
          return cls.node_dict.values()
      
      nodes = nodedom.getElementsByTagName("Node")
      for node_ele in nodes:
          nodeid = int(node_ele.getAttribute("id"))
          nodetype = node_ele.getAttribute("type")
          wuclasses = {}
          wuobjects = {}
          location = ''
          node = WuNode(nodeid, location, wuclasses, wuobjects,type=nodetype) #note: wuclasses, pass by reference, change in original list is also change in node
          if node_ele.hasChildNodes():
              for prop_ele in node_ele.childNodes:
                  if prop_ele.nodeType != prop_ele.ELEMENT_NODE:    
                      continue
                  if prop_ele.tagName == "Location":
                      node.location = prop_ele.getAttribute("content")
		      WuNode.locations[nodeid] = node.location
                  if prop_ele.tagName == "WuClassList":
                      for wuclass_ele in prop_ele.childNodes:
                          if wuclass_ele.nodeType != wuclass_ele.ELEMENT_NODE:    
                              continue
                          wuclass_id = int(wuclass_ele.getAttribute("id"), 0)
                          virtual = True if wuclass_ele.getAttribute("virtual").lower()=="true" else False
                          try:
                              wuclassdef = WuObjectFactory.wuclassdefsbyid[wuclass_id]

                          except KeyError:
                              print '[loadNodes in models] Unknown wuclass id', wuclass_id
                              print ('[loadNodes in models] Invalid saved discover input file', filename)
                              return

                          wuclasses[wuclass_id] = wuclassdef
                  if prop_ele.tagName == "WuObjectList":
                      for wuobj in prop_ele.childNodes:
                          if wuobj.nodeType != wuobj.ELEMENT_NODE:    
                              continue
                          port_number = int(wuobj.getAttribute("port"), 0)
                          wuclass_id = int(wuobj.getAttribute("id"), 0)
                          virtual = True if wuobj.getAttribute("virtual").lower()=="true" else False
                          try:
                              wuclassdef = WuObjectFactory.wuclassdefsbyid[wuclass_id]
                          except KeyError:
                              print '[loadNodes in models] Unknown wuclass id', wuclass_id
                              print ('[loadNodes in models] Invalid saved discover input file', filename)
                              return
                          wuobject = WuObjectFactory.createWuObject(wuclassdef, node, port_number, virtual)
        #                  for wuprop in wuobj.childNodes:
         #                       if wuobj.nodeType != wuobj.ELEMENT_NODE:    
          #                          continue
           #                     prop_name = wuprop.getAttribute("name")
            #                    prop_value = int(wuprop.getAttribute("value"),0)
             #                   wuobject.properties[prop_name] = prop_value
    

      return cls.node_dict.values()                              
      
  def isResponding(self):
    return len(self.wuclasses) > 0 or len(self.wuobjects) > 0


class WuObject:
  ZWAVE_SWITCH_PORT = 64
  def __init__(self, wuclassdef, wunode, port_number, virtual, property_values = None):
    self.port_number = port_number
    self.wuclassdef = wuclassdef
    self.wunode = wunode
    self.virtual = virtual
    self.mapped = 0   #self-identify if a wuobject is mapped, set to True in mapper
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
