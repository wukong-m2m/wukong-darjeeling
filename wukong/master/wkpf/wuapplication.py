import sys, os, traceback, time, re, copy
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
from models import WuClass, WuObject, WuComponent, WuLink, WuType, WuProperty
from mapper import firstCandidate
from locationTree import *
from locationParser import *
from xml.dom.minidom import parse, parseString
from xml.parsers.expat import ExpatError
import simplejson as json
import logging, logging.handlers, wukonghandler
from wkpfcomm import *
from codegen import CodeGen
from xml2java.generator import Generator
from threading import Thread
from subprocess import Popen, PIPE, STDOUT
from collections import namedtuple

from configuration import *
from globals import *

ChangeSets = namedtuple('ChangeSets', ['components', 'links', 'heartbeatgroups'])

class WuApplication:
  def __init__(self, id='', name='', desc='', file='', dir='', outputDir="", templateDir=TEMPLATE_DIR, componentXml=open(COMPONENTXML_PATH).read()):
    self.id = id
    self.name = name
    self.desc = desc
    self.file = file
    self.xml = ''
    self.dir = dir
    self.compiler = None
    self.version = 0
    self.returnCode = 1
    self.status = ""
    self.deployed = False
    self.mapper = None
    self.inspector = None
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

    self.changesets = ChangeSets([], [], [])

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
    self.name = config['name']
    self.desc = config['desc']
    self.dir = config['dir']
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
    return {'id': self.id, 'name': self.name, 'desc': self.desc, 'dir': self.dir, 'xml': self.xml, 'version': self.version}

  def __repr__(self):
    return json.dumps(self.config())

  def parseApplication(self):
      componentInstanceMap = {}
      application_hashed_name = self.applicationDom.getElementsByTagName('application')[0].getAttribute('name')
      # TODO: parse application XML to generate WuClasses, WuObjects and WuLinks
      for index, componentTag in enumerate(self.applicationDom.getElementsByTagName('component')):
          # make sure application component is found in wuClassDef component list
          try:
              assert componentTag.getAttribute('type').lower() in [x.name.lower() for x in WuClass.all()]
          except Exception as e:
            logging.error('unknown types for component found while parsing application')
            return #TODO: need to handle this

          type = componentTag.getAttribute('type')
          location = componentTag.getElementsByTagName('location')[0].getAttribute('requirement')
          group_size = int(componentTag.getElementsByTagName('group_size')[0].getAttribute('requirement'))
          reaction_time = float(componentTag.getElementsByTagName('reaction_time')[0].getAttribute('requirement'))

          action_attributes = {}
          # set default output property values for components in application
          for propertyTag in componentTag.getElementsByTagName('actionProperty'):
            for attr in propertyTag.attributes.values():
              action_attributes[attr.name] = attr.value

          signal_attributes = {}
          # set default input property values for components in application
          for propertyTag in componentTag.getElementsByTagName('signalProperty'):
            for attr in propertyTag.attributes.values():
              signal_attributes[attr.name] = attr.value
          final_attributes = dict(action_attributes.items()
              + signal_attributes.items())

          properties_with_default_values = {}
          for x, y in final_attributes.items():
            if y.strip() != "":
              properties_with_default_values[x] = y

          component = WuComponent(index, location, group_size, reaction_time, type,
                  application_hashed_name, properties_with_default_values)
          componentInstanceMap[componentTag.getAttribute('instanceId')] = component
          self.changesets.components.append(component)

                      
          ''' deprecated
          queries = []
          for locationQuery in componentTag.getElementsByTagName('location'):
              queries.append(locationQuery.getAttribute('requirement'))
          if len(queries) ==0:
              queries.append ('')
          elif len (queries) > 1:
              logging.error('input file violating the assumption there is one location requirement per component in application.xml')
          # nodeId is not used here, portNumber is generated later
          '''

          ''' deprecated
          #TODO: for each component, there is a list of wuObjs (length depending on group_size)
          # Instance id is only for mapping temporarily (since there could be
          # duplicate wuobjects)
          self.wuObjects[wuObj.getInstanceId()] = [wuObj]
          '''

          ''' deprecated
          self.FTComponentPolicy[str(wuObj.getWuClassId())] = {'level': None,
            'reaction': None}

          #FTComponentPolicy
          #assume there is one group_size requirement per component
          for groupSizeQuery in componentTag.getElementsByTagName('group_size'):
              self.FTComponentPolicy[str(wuObj.getWuClassId())]['level'] = int(groupSizeQuery.getAttribute('requirement'))
          #assume there is one reaction_time requirement per component
          for reactionTimeQuery in componentTag.getElementsByTagName('reaction_time'):
              self.FTComponentPolicy[str(wuObj.getWuClassId())]['reaction'] = int(reactionTimeQuery.getAttribute('requirement'))
          '''

      #assumption: at most 99 properties for each instance, at most 999 instances
      linkSet = []  #store hashed result of links to avoid duplicated links: (fromInstanceId*100+fromProperty)*100000+toInstanceId*100+toProperty
      # links
      for linkTag in self.applicationDom.getElementsByTagName('link'):
          from_component_index = componentInstanceMap[linkTag.parentNode.getAttribute('instanceId')].index
          properties = WuClass.where(name=linkTag.parentNode.getAttribute('type'))[0].properties
          from_property_id = [property for property in properties if linkTag.getAttribute('fromProperty').lower() == property.name.lower()][0].id
          
          to_component_index = componentInstanceMap[linkTag.getAttribute('toInstanceId')].index
          
          to_wuclass = WuClass.where(name=componentInstanceMap[linkTag.getAttribute('toInstanceId')].type)[0]
          properties = to_wuclass.properties
          to_property_id = [property for property in properties if linkTag.getAttribute('toProperty').lower() == property.name.lower()][0].id

          to_wuclass_id = to_wuclass.id

          link = WuLink(from_component_index, from_property_id, 
                  to_component_index, to_property_id, to_wuclass_id)
          self.changesets.links.append(link)

          '''
          hash_value = (int(fromInstanceId)*100+int(fromPropertyId))*100000+int(toInstanceId)*100+int(toPropertyId)
          if hash_value not in linkSet:
              linkSet.append(hash_value)
              self.wuLinks.append( WuLink(fromWuObject, fromPropertyId, toWuObject, toPropertyId) )
          '''

  def generateCode(self):
      # special case: for now, it should be passing parsed WuClass objects
      CodeGen.generate(self, open(COMPONENTXML_PATH).read(), ROOT_PATH)

  def generateJava(self):
      Generator.generate(self.name, self.changesets)

  def mapping(self, locTree, routingTable, mapFunc=firstCandidate):
      #input: nodes, WuObjects, WuLinks, WuClassDefs
      #output: assign node id to WuObjects
      # TODO: mapping results for generating the appropriate instiantiation for different nodes
      
      return mapFunc(self, self.changesets, routingTable, locTree)

  def map(self, location_tree, routingTable):
    self.changesets = ChangeSets([], [], [])
    self.parseApplication()
    self.mapping(location_tree, routingTable)
    logging.info("Mapping Results")
    logging.info(self.changesets)

  def deploy_with_discovery(self,*args):
    #node_ids = [info.id for info in getComm().getActiveNodeInfos(force=False)]
    node_ids = set([x.node_id for component in self.changesets.components for x in component.instances])
    self.deploy(node_ids,*args)

  def deploy(self, destination_ids, platforms):
    master_busy()
    app_path = self.dir
    
    for platform in platforms:
      platform_dir = os.path.join(app_path, platform)

      self.status = "Generating java library code"
      gevent.sleep(0)

      # CodeGen
      self.info('==Generating necessary files for wukong')
      try:
        self.generateCode()
      except Exception as e:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        traceback.print_exception(exc_type, exc_value, exc_traceback,
                                      limit=2, file=sys.stdout)
        self.error(e)
        return False

      self.status = "Generating java application"
      gevent.sleep(0)

      # Mapper results, already did in map_application
      # Generate java code
      self.info('==Generating application code in target language (Java)')
      try:
        self.generateJava()
      except Exception as e:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        traceback.print_exception(exc_type, exc_value, exc_traceback,
                                      limit=2, file=sys.stdout)
        self.error(e)
        self.status = "Error generating java application"
        gevent.sleep(0)
        return False

      self.status = "Compressing java to bytecode format"
      gevent.sleep(0)

      # Build the Java code
      self.info('==Compressing application code to bytecode format')
      pp = Popen('cd %s/..; ant clean; ant' % (JAVA_OUTPUT_DIR), shell=True, stdout=PIPE, stderr=PIPE)
      self.returnCode = None
      (infomsg,errmsg) = pp.communicate()

      self.version += 1
      if pp.returncode != 0:
        self.error('==Error generating wkdeploy.dja')
        self.status = "Error generating wkdeploy.dja"
        self.info(infomsg)
        self.error(errmsg)
        gevent.sleep(0)
        return False
      self.info('==Finishing compression')

      self.status = "Deploying bytecode to nodes"
      gevent.sleep(0)

      comm = getComm()
      # Deploy nvmdefault.h to nodes
      self.info('==Deploying to nodes %s' % (str(destination_ids)))
      remaining_ids = copy.deepcopy(destination_ids)

      gevent.sleep(0)
      for node_id in destination_ids:
        remaining_ids.remove(node_id)
        self.status = "Deploying bytecode to node %d, remaining %s" % (node_id, str(remaining_ids))
        self.info('==Deploying to node id: %d' % (node_id))
        if not comm.reprogram(node_id, os.path.join(JAVA_OUTPUT_DIR, '..', 'build', 'wkdeploy.dja'), retry=3):
          self.status = "Deploy unsucessful for node %d" % (node_id)
          self.error('==Node not deployed successfully')
          return False
        self.info('...completed')
    self.info('==Deployment has completed')
    self.status = "Deployment has succeeded"
    self.status = "clear" # close the dialog
    master_available()
    return True

  def reconfiguration(self):
    global location_tree
    global routingTable
    master_busy()
    self.status = "Start reconfiguration"
    node_infos = getComm().getActiveNodeInfos(force=True)
    location_tree = LocationTree(LOCATION_ROOT)
    location_tree.buildTree(node_infos)
    routingTable = getComm().getRoutingInformation()
    self.map(location_tree, routingTable)
    self.deploy([info.id for info in node_infos], DEPLOY_PLATFORMS)
    master_available()

