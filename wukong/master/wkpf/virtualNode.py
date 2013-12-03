import gevent
import imp
from models import *

# The base class for user programmed python wuclass
class VirtualNodeWuClass:
  def __init__(self, port_number, node_id):
    self.port_number = port_number
    self.properties = VirtualNodeProperties()
    self.node_id = node_id

class VirtualNodeProperties:
  def __init__(self, properties=[]):
    self.properties = properties

  def __iter__(self):
    for elem in self.properties:
      yield elem

  def get(key):
    for property in self.properties:
      if property.key == key:
        return property.get()

  def set(key, value):
    for property in self.properties:
      if property.key == key:
        return property.set(value)


class VirtualNodeProperty:
  def __init__(self, key, number):
    self.number = number # the number from WuPropertyDef in models.py
    self.key = key
    self.dirty = false
    self.value = None

  # FIXME: will have to add type checking
  def set(self, value):
    self.dirty = true
    self.value = value

  def get(self):
    return self.value


class VirtualNodeLink:
  def __init__(self, component, property):
    self.component = component
    self.property = property # the linked property


# the parsed WuLinks of reference WuComponent for virtual node wuclasses
# FIXME: find a better way to set reference component
class VirtualNodeLinks:
  def __init__(self, components, links, component): 
    self.components = components # changesets.components
    self.links = links # changesets.links
    self.component = component # reference component 

  def outdegrees(self):
    outdegress = []
    for link in self.links:
      if link.from_component_index == component.index:
        outdegress_component = link.outdegress_component_index
        outdegress_instances = outdegress_component.instances # all mapped wuobjects
        for instance in outdegress_instances:
          outdegress.append(VirtualNodeLink(outdegress_component, VirtualNode().getPropertiesOfInstance(instance)))
    return outdegress

  def indegrees(self):
    indegress = []
    for link in self.links:
      if link.to_component_index == component.index:
        indegress_component = link.indegress_component_index
        indegress_instances = indegress_component.instances # all mapped wuobjects
        for instance in indegress_instances:
          indegress.append(VirtualNodeLink(indegress_component, VirtualNode().getPropertiesOfInstance(instance)))
    return indegress




class VirtualNode:
  @classmethod
  def getWuClassDefs(cls):
    return [WuClassDef.find(id=434)]

  _virtualNode = None
  @classmethod
  def init(cls):
    if not cls._virtualNode:
      cls._virtualNode = VirtualNode()
    return cls._virtualNode

  def __init__(self):
    self.wuobjects = {}
    self.component = []
    self.links = []
    self.changesets = None
    imp.load_source('virtualNWuClass', '../virtual_node/wuclasses')
    gevent.spawn(self.run)

  def setComm(self,comm):
    self.comm = comm

  # Create real in memory wuobjects (not the wuobjects in models for mapping)
  def initializeWuObjects(self, changesets):
    self.wuobjects = {}
    for component in changesets.components:
      for instance in component.instances:
        wuclassdef = instance.wuclassdef()
	if wuclassdef.name and wuclassdef.id:
	  # metaprogramming
	  # eval is not secure, but we can be pretty sure that security is not on our priority list for now
          wuobject = eval(wuclassdef.name)()
          wuobject.node_id = instance.wunode().id
          wuobject.port_number = instance.port_number
          wuobject.properties = VirtualNodeProperties([VirtualNodeProperty(wpds.name, wpds.number) for wpds in wuclassdef.wupropertydefs()])
          self.wuobjects[instance] = wuobject

  def setupWuObjects(self):
    for wuobject in self.wuobjects.values():
      wuobject.setup()

  def deploy(self, changesets):
    self.changesets = changesets # just holding reference
    self.initializeWuObjects(self.changesets)
    self.components = self.changesets.components
    self.links = changesets.links
    self.setupWuObjects()

  def updateWuObjects(self):
    for wuobject in self.wuobjects.values():
      wuclassdef = WuClassDef.find(name=wuobject.__class__.__name__)
      for search in self.components:
        if search.type == wuclassdef.name:
          component = search
      wuobject.update(wuobject.properties, VirtualNodeLinks(self.components, self.links, component))

  def findComponentFromIndex(components, index):
    for search in components:
      if search.index == index:
        return search

    return None

  def propagateLinks(self):
    if self.changesets:
      # Get all component indexes where they are on virtual node
      indexes = []
      for ind, component in enumerate(self.changesets.components):
        component_wuclass = WuClassDef.find(name=component.type)
        if component_wuclass.id in [x.id for x in VirtualNode.getWuClassDefs()]:
          indexes.append(ind)

      # For this demo we are only concerned with outgoing links from virtual node
      for link in [link for link in self.changesets.links if link.from_component_index in indexes]:

        from_component = findComponentFromIndex(self.components, link.to_component_index)
        to_component = findComponentFromIndex(self.components, link.to_component_index)

        # FIXME: Assuming one to many (so MCHESS to many light wuobjects)
        from_dirty_properties = [property for property in self.wuobjects[from_component.instances[0]].properties if property.dirty]
        for dirty_property in from_dirty_properties:
          if link.from_property_id == dirty_property.number:
            for instance in to_component.instances:
              node = instance.wunode() # WuNode
              self.pushProperty(node, instance, link.to_property_id, dirty_property.value)
              dirty_property.dirty = false
       	      gevent.sleep(0.01)
        
          
        #testing
        #self.setProperty(link.to_property_id, self.changesets.components[link.to_component_index], 0)
        #gevent.sleep(1) # sleep for 1 sec
        #self.setProperty(link.to_property_id, self.changesets.components[link.to_component_index], 255)
        #gevent.sleep(1) # sleep for 1 sec
    
  def run(self):
    while 1:
      # update all wuobjects to update properties, sense or actuate
      self.updateWuObjects()

      # propagate changed properties to other properties based on the links
      self.propagateLinks()
      gevent.sleep(1) # sleep for 1 sec

  def getPropertiesOfInstance(self, instance):
    return self.wuobjects[instance].properties


  #Will push dirty properties to local or remote node
  #Not used in this demo
  def pushProperty(self, node, instance, property_id, value):
    pass
