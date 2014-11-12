import os, sys
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
import copy
from xml.etree import ElementTree
import xml.dom.minidom
import traceback
from xml.dom.minidom import parse, parseString
import logging, logging.handlers, wukonghandler
from xml.parsers.expat import ExpatError
from collections import namedtuple
import simplejson as json
from configuration import *
import logging

ChangeSets = namedtuple('ChangeSets', ['components', 'links', 'heartbeatgroups'])

# Copy from original WuApplication, split the deployment functionality out into ApplicationManager.
# At the same time, add some data fields for energy aware mapping
class WuApplication:
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
        self.status = "" # deprecated, replaced by wukong_status and deploy_status
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

    def getReturnCode(self):
        return self.returnCode

    def getStatus(self):
        return self.status

    def config(self):
        return {'id': self.id, 'app_name': self.app_name, 'desc': self.desc, 'dir': self.dir, 'xml': self.xml, 'version': self.version}

    def __repr__(self):
        return json.dumps(self.config())

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
  def __init__(self, id, location, wuclassdefs=None, wuobj=None,energy=100.0,type='wudevice'):
    self.id = id
    if location == None:
	# try:
	#    self.location = WuNode.locations[id]
	# except:
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
  
  def dump(self):   
    stri = ''
    stri += "id:"+str(self.id)+'\n'
    stri += "location:"+ self.location+'\n'
    stri += "wuclasses:" +str(self.wuclasses)+'\n'
    stri += "wuobjects"+str(self.wuobjects)+'\n'
    stri += "type:"+self.type+'\n'
    return stri

  @staticmethod
  def addNodes(nodes):
    for node in nodes:
        WuNode.node_dict[node.id] =  node
    return

  @staticmethod
  def addVirtualNodes(nodes):
    WuNode.node_dict =  dict(WuNode.node_dict.items() +  nodes.items())
    return

  @staticmethod
  def getAllWuNodes():
    return WuNode.node_dict.values()
      
  def isResponding(self):
    return len(self.wuclasses) > 0 or len(self.wuobjects) > 0


class WuObject:
  ZWAVE_SWITCH_PORT = 64
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
