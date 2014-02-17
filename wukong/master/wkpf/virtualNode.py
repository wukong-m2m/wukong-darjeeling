import sys, os
import gevent
from models import *
from virtualNodeWuClass import *

# Helper method
def findComponentFromIndex(components, index):
  for search in components:
    if search.index == index:
      return search

  return None

def keyFromInstance(instance):
  return (instance.port_number, instance.wunode.id, instance.wuclassdef.id)




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
    outdegrees = []
    for link in self.links:
      if link.from_component_index == self.component.index:
        outdegrees_component = findComponentFromIndex(self.components, link.to_component_index)
        outdegrees_instances = outdegrees_component.instances # all mapped wuobjects
        for instance in outdegrees_instances:
          wuclassdef = WuObjectFactory.wuclassdefsbyname[outdegrees_component.type]
          wupropertydef = WuObjectFactory.wuclassdefsbyid[wuclassdef.id].properties[link.to_property_name]
          outdegrees.append(VirtualNodeLink(outdegrees_component, VirtualNode.init().getPropertiesOfInstance(instance).getProperty(wupropertydef.name)))
    return outdegrees

  def indegrees(self):
    indegrees = []
    for link in self.links:
      if link.to_component_index == component.index:
        indegrees_component = findComponentFromIndex(self.components, link.from_component_index)
        indegrees_instances = indegrees_component.instances # all mapped wuobjects
        for instance in indegress_instances:
          wuclassdef = WuObjectFactory.wuclassdefsbyname[indegrees_component.type]
          wupropertydef = WuObjectFactory.wuclassdefsbyid[wuclassdef.id].properties[link.from_property_name]
          indegrees.append(VirtualNodeLink(indegrees_component, VirtualNode.init().getPropertiesOfInstance(instance).getProperty(wupropertydef.name)))
    return indegrees




class VirtualNode:
  @classmethod
  def getWuClassDefs(cls):
    return [WuObjectFactory.wuclassdefsbyid[434]]

  _virtualNode = None
  @classmethod
  def init(cls):
    if not cls._virtualNode:
      cls._virtualNode = VirtualNode()
    return cls._virtualNode

  def __init__(self):
    print 'VirtualNode __init__'
    self.wuobjects = {}
    self.components = []
    self.links = []
    self.changesets = None

    '''
    import imp
    from os import listdir
    from os.path import isfile, join
    mypath = '../virtual_node/wuclasses'
    for py in [f for f in listdir(mypath) if isfile(join(mypath, f))]:
      print py
      print os.path.abspath(join(mypath, py))
      if py.endswith('py'):
        py_mod = imp.load_source(py, os.path.abspath(join(mypath, py)))
        print py_mod
        from py import *
    '''
    print 'VirtualNode imp load_source'
    #gevent.spawn(self.run)

  def setComm(self,comm):
    self.comm = comm

  # Create real in memory wuobjects (not the wuobjects in models for mapping)
  def initializeWuObjects(self, changesets):
    print 'initializing WuObjects'
    for component in changesets.components:
      for instance in component.instances:
        wuclassdef = instance.wuclassdef
        if wuclassdef.name and wuclassdef.id:
          # metaprogramming
          # eval is not secure, but we can be pretty sure that security is not on our priority list for now
          properties = VirtualNodeProperties([VirtualNodeProperty(wpds.name, wpds.number) for wpds in wuclassdef.properties])

          # set default values
          for propertydef in wuclassdef.properties:
            value = propertydef.value
            # FIXME: hard code datatype
            if propertydef.name == 'level':
              value = int(propertydef.value)
            if value == 'true':
              value = True
            if value == 'false':
              value = False
            properties.set(propertydef.name, value)

          wuobject = eval(wuclassdef.name)(instance.port_number, instance.wunode.id, properties)
          print 'putting instance ', keyFromInstance(instance)
          self.wuobjects[keyFromInstance(instance)] = wuobject

  def setupWuObjects(self):
    print 'setuping WuObjects'
    for wuobject in self.wuobjects.values():
      print 'setup WuObject', wuobject
      wuobject.setup()

  def deploy(self, changesets):
    return
    self.changesets = changesets # just holding reference
    self.initializeWuObjects(self.changesets)
    self.components = self.changesets.components
    self.links = changesets.links
    self.setupWuObjects()

  def updateWuObjects(self):
    if self.changesets and self.components:
      for wuobject in self.wuobjects.values():
        wuclassdef = WuObjectFactory.wuclassdefsbyname[wuobject.__class__.__name__]
        for search in self.components:
          if search.type == wuclassdef.name:
            component = search
        wuobject.update(self, wuobject.properties, VirtualNodeLinks(self.components, self.links, component))
        gevent.sleep(0.01)

  def propagateLinks(self):
    if self.changesets and self.components:
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
        if keyFromInstance(from_component.instances[0]) in self.wuobjects:
          from_dirty_properties = [property for property in self.wuobjects[keyFromInstance(from_component.instances[0])].properties if property.dirty]
          for dirty_property in from_dirty_properties:
            if link.from_property_id == dirty_property.number:
              for instance in to_component.instances:
                node = instance.wunode() # WuNode
                self.pushProperty(node, instance, link.to_property_id, dirty_property.value)
                dirty_property.dirty = False
                gevent.sleep(0.01)
        
          
        #testing
        #self.setProperty(link.to_property_id, self.changesets.components[link.to_component_index], 0)
        #gevent.sleep(1) # sleep for 1 sec
        #self.setProperty(link.to_property_id, self.changesets.components[link.to_component_index], 255)
        #gevent.sleep(1) # sleep for 1 sec
    
  def run(self):
    while 1:
      if self.changesets and self.components:
        # update all wuobjects to update properties, sense or actuate
        self.updateWuObjects()

        # propagate changed properties to other properties based on the links
        self.propagateLinks()
      gevent.sleep(1) # sleep for 1 sec

  def getPropertiesOfInstance(self, instance):
    print 'getPropertiesOfInstance', self.wuobjects
    return self.wuobjects[keyFromInstance(instance)].properties


  #Will push dirty properties to local or remote node
  #Not used in this demo
  def pushProperty(self, node, instance, property_id, value):
    pass

sys.path.append(os.path.abspath('../virtual_node'))
from wuclasses import *


if __name__ == '__main__':
  VirtualNode.init()
  print Threshold(1, 1)
  print MCHESS(1, 1)
  print Dimmer(1, 1)
