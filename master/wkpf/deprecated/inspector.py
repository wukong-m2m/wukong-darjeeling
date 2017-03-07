#!/usr/bin/python
from optparse import OptionParser
import xml.dom.minidom
import os
from wkpfcomm import *
from wkpf.parser import *
import wkpf.pynvc
import copy

rootpath = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")

def find(seq, f):
  """Return first item in sequence where f(item) == True."""
  for item in seq:
    if f(item): 
      return item
  return None

## Component XML functions
def getComponentDefinitions(path):
  return xml.dom.minidom.parse(path)

def getComponent(wuClassId, componentDefinitions):
  for component in componentDefinitions.getElementsByTagName('WuClass'):
    if int(component.getAttribute('id'),0) == wuClassId:
      return component
  raise Error('No component found with id %d' % (wuClassId))

def getComponentPropertyCount(wuClassId, componentDefinitions):
  component = getComponent(wuClassId, componentDefinitions)
  return len(component.getElementsByTagName('property'))

def getComponentName(wuClassId, componentDefinitions):
  component = getComponent(wuClassId, componentDefinitions)
  return component.getAttribute('name')

def getComponentPropertyName(wuClassId, propertyNumber, componentDefinitions):
  component = getComponent(wuClassId, componentDefinitions)
  prop = component.getElementsByTagName('property')[propertyNumber]
  return prop.getAttribute('name')

def stringRepresentationIfEnum(wuClassId, propertyNumber, componentDefinitions, value):
  component = getComponent(wuClassId, componentDefinitions)
  prop = component.getElementsByTagName('property')[propertyNumber]
  datatype = prop.getAttribute('datatype')
  for typedef in componentDefinitions.getElementsByTagName('WuTypedef'):
    if typedef.getAttribute('name') == datatype:
      enums = typedef.getElementsByTagName('enum')
      return enums[value].getAttribute('value')        
  return str(value)

## Flow XML functions
def getFlow(path):
  return xml.dom.minidom.parse(path)

def getRemoteLinks(propertyInfo, wuObjectInfo, flowDefinition, mapping, componentDefinitions):
  componentInstanceName = getComponentInstanceName(wuObjectInfo.id, wuObjectInfo.port_number, mapping)
  propertyName = propertyInfo.name
  #propertyName = getComponentPropertyName(wuObjectInfo.getWuClassId(), propertyInfo.getId(), componentDefinitions)
  links = []
  for component in flowDefinition.getElementsByTagName('component'):
    for link in component.getElementsByTagName('link'):
      if component.getAttribute('instanceId') == componentInstanceName and link.getAttribute('fromProperty') == propertyName: # Output side of a link
        links.append(('=>', link.getAttribute('toInstanceId'), link.getAttribute('toProperty')))
      if link.getAttribute('toInstanceId') == componentInstanceName and link.getAttribute('toProperty') == propertyName: # Input side of a link
        links.append(('<=', component.getAttribute('instanceId'), link.getAttribute('fromProperty')))
  return links

## Mapping functions
def getMapping(path):
  return xml.dom.minidom.parse(path)

def getComponentInstanceName(nodeId, portNumber, mapping):
  entries = mapping.getElementsByTagName('entry')
  for entry in entries:
    if entry.getAttribute('nodeid') == str(nodeId) and entry.getAttribute('portnumber') == str(portNumber):
      return entry.getAttribute('componentname')
  return None # Not mapped to any component in this application

def getNodeAndPortForComponent(componentName, mapping):
  entries = mapping.getElementsByTagName('entry')
  for entry in entries:
    if entry.getAttribute('componentname') == componentName:
      return int(entry.getAttribute('nodeid')), int(entry.getAttribute('portnumber'))
  raise Error('Component not found: %s', (componentName))

## Functions to read data from the nodes
def readPropertyInfo(wuObject, property):
  wkpfcommData = comm.getProperty(wuObject, property.id)
  property.value = wkpfcommData[0]
  property.status = wkpfcommData[2]
  property.save()

def readNodeInfo(nodeId, componentDefinitions):
  wkpfcommNodeInfo = comm.getNodeInfo(nodeId)
  if wkpfcommNodeInfo.isResponding():
    for wuObject in wkpfcommNodeInfo.wuobjects:
      properties = wuObject.wuclass.properties
      for property in properties:
        readPropertyInfo(wuObject, property)
  return wkpfcommNodeInfo

## Print functions
def printNodeInfos(nodeInfos, componentDefinitions, flowDefinition, mapping):
  for nodeInfo in sorted(nodeInfos, key=lambda x:x.id):
    print "============================="
    print "NodeID: %d" % (nodeInfo.id)
    if not nodeInfo.isResponding():
      print "NOT RESPONDING"
    else:
      print "WuClasses:"
      def pt(wuClassInfo): 
        print wuClassInfo
      map(pt, sorted(nodeInfo.wuclasses, key=lambda x:x.id))
      print "WuObjects:"
      for wuObjectInfo in sorted(nodeInfo.wuobjects, key=lambda x:x.wuclass.id):
        if mapping:
          componentInstanceName = getComponentInstanceName(nodeInfo.id, wuObjectInfo.port_number, mapping)
          if componentInstanceName:
            print "\tComponent instance name: %s" % (componentInstanceName)
        print "\tClass name %s" % (wuObjectInfo.wuclass.name)
        print "\tPort number %d" % (wuObjectInfo.port_number)
        print "\tPropNr  Datatype         Name      value (status) [(remote property value, status)]"
        for propertyInfo in wuObjectInfo.wuclass.properties:
          propertyInfoString = "\t   %d %8s %15s %10s (%s)" % (propertyInfo.id,
                                               #wkpf.datatypeToString(propertyInfo.getDataType()),
                                               propertyInfo.datatype,
                                               propertyInfo.name,
                                               #stringRepresentationIfEnum(wuObjectInfo.getWuClassId(), propertyInfo.getId(), componentDefinitions, propertyInfo.getCurrentValue()),
                                               propertyInfo.value,
                                               propertyInfo.status)
          remoteLinkAndValue = ""
          if mapping and flowDefinition:
            remoteLinks = getRemoteLinks(propertyInfo, wuObjectInfo, flowDefinition, mapping, componentDefinitions)
            for remoteLink in remoteLinks:
              try:
                remoteNodeId, remotePortNumber = getNodeAndPortForComponent(remoteLink[1], mapping)
                remoteNodeInfo = find(nodeInfos, lambda x:x.id == remoteNodeId)
                if remoteNodeInfo == None:
                  propertyInfoString += " NODE NOT FOUND"
                elif not remoteNodeInfo.isResponding():
                  propertyInfoString += " NODE %d NOT RESPONDING" % (remoteNodeInfo.id)
                else:
                  remoteWuObject = find(remoteNodeInfo.wuobjects, lambda x:x.port_number == remotePortNumber)
                  remotePropertyInfo = find(remoteWuObject.wuclass.properties, lambda x:x.name() == remoteLink[2])
                  propertyInfoString += "   (%s %s %s = %s, %0#4x)" % (remoteLink[0], remoteLink[1], remoteLink[2], remotePropertyInfo.value, remotePropertyInfo.access)
                  if propertyInfo.value != remotePropertyInfo.value:
                    propertyInfoString += " !!!!"
              except:
                propertyInfoString += " REMOTE PROPERTY NOT FOUND!!!!"
          print propertyInfoString
        print ""

class Inspector:
  def __init__(self, changesets):
    self.changesets = changesets
    self.comm = Communication(0)
    self.node_infos = comm.getAllNodeInfos()

  def readAllLog(self):
    logs = []
    for node_info in self.node_infos:
      pass

  def readLog(self, node_id):
    logs = []
    if node_id and node_id in [info.id for info in self.node_infos]:
      pass

  def inspectAllProperties():
    properties = []
    for component in self.changesets.components:
      for wuobject in component.instances:
        properties += wuobject.properties

    for property in properties:
      for node_info in node_infos:
        if node_info.isResponding():
          node_info.wuObjects
        else:
          properties.append()

if __name__ == "__main__":
  optionParser = OptionParser("usage: %prog [options]")
  optionParser.add_option("-c", "--component", action="store", type="string", dest="pathComponentXml", help="WuKong Component XML file path (default NVMROOT/ComponentDefinitions/WuKongStandardLibrary.xml)")
  optionParser.add_option("-m", "--mapping", action="store", type="string", dest="pathMappingXml", help="Mapping XML file path (default NVMROOT/vm/build/avr_mega2560/currentMapping.xml)")
  optionParser.add_option("-f", "--flow", action="store", type="string", dest="pathFlowXml", help="WuKong Flow XML file path (default NVMROOT/vm/build/avr_mega2560/currentFlow.xml)")
  (options, args) = optionParser.parse_args()
  
  if not options.pathComponentXml and os.path.exists(rootpath + "/ComponentDefinitions/WuKongStandardLibrary.xml"):
    options.pathComponentXml = rootpath + "/ComponentDefinitions/WuKongStandardLibrary.xml"
  if not options.pathMappingXml and os.path.exists(rootpath + "/vm/build/avr_mega2560/currentMapping.xml"):
    options.pathMappingXml = rootpath + "/vm/build/avr_mega2560/currentMapping.xml"
  if not options.pathFlowXml and os.path.exists(rootpath + "/vm/build/avr_mega2560/currentFlow.xml"):
    options.pathFlowXml = rootpath + "/vm/build/avr_mega2560/currentFlow.xml"
  if not options.pathComponentXml:
    optionParser.error("invalid component xml, please refer to -h for help")

  comm = getComm()
  Parser.parseLibrary(options.pathComponentXml)
  componentDefinitions = getComponentDefinitions(options.pathComponentXml)
  mapping = getMapping(options.pathMappingXml) if options.pathMappingXml else None
  flowDefinition = getFlow(options.pathFlowXml) if options.pathFlowXml else None
  nodeIds = comm.getNodeIds();
  nodeInfos = [readNodeInfo(nodeId, componentDefinitions) for nodeId in nodeIds]
  
  print "============================="
  print "Component xml path: %s" % (options.pathComponentXml)
  print "Mapping xml path: %s" % (options.pathMappingXml)
  print "Flow xml path: %s" % (options.pathFlowXml)
  if flowDefinition:
    print "Application in flow xml: %s" % flowDefinition.getElementsByTagName('application')[0].getAttribute('name')
  printNodeInfos(nodeInfos, componentDefinitions, flowDefinition, mapping)


