import copy
from xml.etree import ElementTree
import xml.dom.minidom
import traceback
from wkpf import globals
from xml.dom.minidom import parse, parseString
from xml.parsers.expat import ExpatError
import simplejson as json
from configuration import *
import logging
import simplejson as json

# It is a global view of the overall system, like devices, Wuclasses, WuObjects, Energy Consumptions on device.
class WuSystem:
    __virtual_nodes = None
    __mapping_results = {}
    @classmethod
    def getVirtualNodes(cls):
        # 20141128 Refactored globals.virtual_nodes and initializeVirtualNodes to a method that just returns
        # the same dictionary of nodes each time.
        if cls.__virtual_nodes == None or True:
            cls.__virtual_nodes = {}
            # add the virtual wunode just used to represent master which is the destination of monitoring link
            wuclasses = {}
            wuobjects = {}

            # 1 is by default the network id of the controller
            node = WuNode(WUKONG_GATEWAY, '/' + LOCATION_ROOT, wuclasses, wuobjects, 'virtualdevice')
            wuclassdef = WuObjectFactory.wuclassdefsbyid[44]
            wuobject = WuObjectFactory.createWuObject(wuclassdef, node, 1, False)
            cls.__virtual_nodes[1] = node
        return cls.__virtual_nodes

    @classmethod
    def hasMappingResult(cls, appId):
        return appId in cls.__mapping_results

    @classmethod
    def dumpAllResult(cls):
        logging.info(json.dumps(cls.__mapping_results))
        return cls.__mapping_results

    @classmethod
    def addMappingResult(cls, appId, result):
        cls.__mapping_results[appId] = result

    @classmethod
    def lookUpComponent(cls, appId, componentId):
        return cls.__mapping_results[appId][componentId]

# Copy from original WuApplication, split the deployment functionality out into ApplicationManager.
# At the same time, add some data fields for energy aware mapping
class WuApplicationNew:
    def __init__(self, id='', app_name='', desc='', file='', dir='', outputDir="", templateDir=TEMPLATE_DIR, componentXml=open(COMPONENTXML_PATH).read()):
        self.id = id
        self.app_name = app_name
        self.desc = desc
        self.file = file
        self.xml = ''
        self.dir = dir
        self.version = 0
        self.returnCode = 1
        self.deployed = False
        # 5 levels: self.logger.debug, self.logger.info, self.logger.warn, self.logger.error, self.logger.critical
        self.logger = logging.getLogger(self.id[:5])
        self.logger.setLevel(logging.DEBUG) # to see all levels
        self.loggerHandler = wukonghandler.WukongHandler(1024 * 3, target=logging.FileHandler(os.path.join(self.dir, 'compile.log')))
        self.logger.addHandler(self.loggerHandler)

        # For Mapper
        self.name = ""
        self.applicationDom = ""
        self.destinationDir = outputDir
        self.templateDir = templateDir
        self.componentXml = componentXml
        self.wuComponents = {}
        self.instanceIds = []
        self.monitorProperties = {}

        self.changesets = ChangeSets([], [], [])

        # a log of mapping results warning or errors
        # format: a list of dict of {'msg': '', 'level': 'warn|error'}
        self.mapping_status = []

        # a log of deploying results warning or errors
        # format: a list of dict of {'msg': '', 'level': 'warn|error'}
        self.deploy_status = []
        self.deploy_ops = ''

    def clearMappingStatus(self):
        self.mapping_status = []

    def errorMappingStatus(self, msg):
        self.mapping_status.append({'msg': msg, 'level': 'error'})

    def warnMappingStatus(self, msg):
        self.mapping_status.append({'msg': msg, 'level': 'warn'})

    def clearDeployStatus(self):
        self.deploy_status = []
        self.deploy_ops = ''
        print 'clear deploy status.........'

    # signal to client to stop polling
    def stopDeployStatus(self):
        self.deploy_ops = 'c'

    def logDeployStatus(self, msg):
        self.info(msg)
        self.deploy_status.append({'msg': msg, 'level': 'log'})

    def errorDeployStatus(self, msg):
        self.error(msg)
        self.deploy_status.append({'msg': msg, 'level': 'error'})

    def warnDeployStatus(self, msg):
        self.warning(msg)
        self.deploy_status.append({'msg': msg, 'level': 'warn'})

    def setFlowDom(self, flowDom):
        self.applicationDom = flowDom
        self.name = flowDom.getElementsByTagName('application')[0].getAttribute('name')

    def setOutputDir(self, outputDir):
        self.destinationDir = outputDir

    def setTemplateDir(self, templateDir):
        self.templateDir = templateDir

    def setComponentXml(self, componentXml):
        self.componentXml = componentXml

    def logs(self):
        self.loggerHandler.retrieve()
        logs = open(os.path.join(self.dir, 'compile.log')).readlines()
        return logs

    def retrieve(self):
        return self.loggerHandler.retrieve()

    def info(self, line):
        self.logger.info(line)
        self.version += 1

    def error(self, line):
        self.logger.error(line)
        self.version += 2

    def warning(self, line):
        self.logger.warning(line)
        self.version += 1

    def updateXML(self, xml):
        self.xml = xml
        self.setFlowDom(parseString(self.xml))
        self.saveConfig()
        f = open(os.path.join(self.dir, self.id + '.xml'), 'w')
        f.write(xml)
        f.close()

    def loadConfig(self):
        config = json.load(open(os.path.join(self.dir, 'config.json')))
        self.id = config['id']
        try:
            self.app_name = config['app_name']
        except:
            self.app_name='noname';
            self.desc = config['desc']
            # self.dir = config['dir']
            self.xml = config['xml']
        try:
            dom = parseString(self.xml)
            self.setFlowDom(dom)
        except ExpatError:
            pass

    def saveConfig(self):
        json.dump(self.config(), open(os.path.join(self.dir, 'config.json'), 'w'))

    def getReturnCode(self):
        return self.returnCode

    def getStatus(self):
        return self.status

    def config(self):
        return {'id': self.id, 'app_name': self.app_name, 'desc': self.desc, 'dir': self.dir, 'xml': self.xml, 'version': self.version}

    def __repr__(self):
        return json.dumps(self.config())

    def parseApplication(self):
        componentInstanceMap = {}
        wuLinkMap = {}
        application_hashed_name = self.applicationDom.getElementsByTagName('application')[0].getAttribute('name')

        components = self.applicationDom.getElementsByTagName('component')
        self.instanceIds = []

        # parse application XML to generate WuClasses, WuObjects and WuLinks
        for componentTag in components:
            # make sure application component is found in wuClassDef component list
            try:
                assert componentTag.getAttribute('type') in WuObjectFactory.wuclassdefsbyname.keys()
            except Exception as e:

                logging.error('unknown types for component found while parsing application')
                return #TODO: need to handle this

            type = componentTag.getAttribute('type')

            if componentTag.getElementsByTagName('location'):
                location = componentTag.getElementsByTagName('location')[0].getAttribute('requirement')
            else:
                location = '/'+LOCATION_ROOT

            if componentTag.getElementsByTagName('group_size'):
                group_size = int(componentTag.getElementsByTagName('group_size')[0].getAttribute('requirement'))
            else:
                group_size = 1

            if componentTag.getElementsByTagName('reaction_time'):
                reaction_time = float(componentTag.getElementsByTagName('reaction_time')[0].getAttribute('requirement'))
            else:
                reaction_time = 2.0

            properties = {}
            # set default output property values for components in application
            for propertyTag in componentTag.getElementsByTagName('actionProperty'):
                for attr in propertyTag.attributes.values():
                    properties[attr.name] = attr.value.strip()

            # set default input property values for components in application
            for propertyTag in componentTag.getElementsByTagName('signalProperty'):
                for attr in propertyTag.attributes.values():
                    properties[attr.name] = attr.value.strip()


            index = componentTag.getAttribute('instanceId')
            self.monitorProperties[index] = {}

            # set monitoring properties index for components in application
            for propertyTag in componentTag.getElementsByTagName('monitorProperty'):
                for attr in propertyTag.attributes.values():
                    self.monitorProperties[index][attr.name] = attr.value.strip()

            if index in self.instanceIds:

                #wucomponent already appears in other pages, merge property requirement, suppose location etc are the same
                self.wuComponents[index].properties = dict(self.wuComponents[index].properties.items() + properties.items())
            else:
                component = WuComponent(index, location, group_size, reaction_time, type, application_hashed_name, properties)
                componentInstanceMap[componentTag.getAttribute('instanceId')] = component
                self.wuComponents[componentTag.getAttribute('instanceId')] = component
                self.changesets.components.append(component)
                self.instanceIds.append(index)

        # add server as component in node 0
        component = WuComponent(1, '/'+LOCATION_ROOT, 1, 2.0, 'Server', 0, {})
        componentInstanceMap[0] = component
        self.wuComponents[0] = component
        self.changesets.components.append(component)
        self.instanceIds.append(0)
        #assumption: at most 99 properties for each instance, at most 999 instances
        #store hashed result of links to avoid duplicated links: (fromInstanceId*100+fromProperty)*100000+toInstanceId*100+toProperty
        linkSet = []
        # links
        for linkTag in self.applicationDom.getElementsByTagName('link'):
            from_component_id = linkTag.parentNode.getAttribute('instanceId')
            from_component = componentInstanceMap[from_component_id]
            from_property_name = linkTag.getAttribute('fromProperty').lower()
            from_property_id = WuObjectFactory.wuclassdefsbyname[from_component.type].properties[from_property_name].id
            to_component_id = linkTag.getAttribute('toInstanceId')
            to_component = componentInstanceMap[to_component_id]
            to_property_name =  linkTag.getAttribute('toProperty').lower()
            to_property_id = WuObjectFactory.wuclassdefsbyname[to_component.type].properties[to_property_name].id

            hash_value = (int(from_component_id)*100+int(from_property_id))*100000+int(to_component_id)*100+int(to_property_id)
            if hash_value not in wuLinkMap.keys():
                link = WuLink(from_component, from_property_name,
                        to_component, to_property_name)
                wuLinkMap[hash_value] = link
            self.changesets.links.append(wuLinkMap[hash_value])

        #add monitoring related links
        if(MONITORING == 'true'):
            for instanceId, properties in self.monitorProperties.items():
                for name in properties:
                    hash_value = (int(instanceId)*100 + int(properties[name])*100000 + 0 + 0)
                    if hash_value not in wuLinkMap.keys():
                        link = WuLink(componentInstanceMap[instanceId], name, componentInstanceMap[0], 'input')
                        wuLinkMap[hash_value] = link
                    self.changesets.links.append(wuLinkMap[hash_value])

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

  @staticmethod
  def addVirtualNodes(nodes):
    WuNode.node_dict =  dict(WuNode.node_dict.items() +  nodes.items())
    return

  @staticmethod
  def getAllWuNodes():
    return WuNode.node_dict.values()

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

      #add the virtual nodes
      cls.node_dict = dict(cls.node_dict.items() + WuSystem.getVirtualNodes().items())
      return cls.node_dict.values()                              
      
  def isResponding(self):
    return len(self.wuclasses) > 0 or len(self.wuobjects) > 0


class WuObject:
  ZWAVE_SWITCH_PORT1 = 64
  ZWAVE_SWITCH_PORT2 = 65
  ZWAVE_SWITCH_PORT3 = 66
  ZWAVE_DIMMER1 = 67
  ZWAVE_DIMMER2 = 68
  ZWAVE_DIMMER3 = 69
  def __init__(self, wuclassdef, wunode, port_number, virtual, property_values = None):
    self.port_number = port_number
    self.wuclassdef = wuclassdef
    self.wunode = wunode
    self.virtual = virtual
    self.created = False
    self.mapped = False   #self-identify if a wuobject is mapped, set to True in mapper
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
