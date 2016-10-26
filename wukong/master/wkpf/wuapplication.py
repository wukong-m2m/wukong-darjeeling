# vim:ts=2 sw=2 expandtab
#
# 2015 July 10, HY modified
#   Add .disable property to WuApplication
#
import sys, os, traceback, time, re, copy
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
from model.models import WuClassDef, WuComponent, WuLink
from mapper.mapper import firstCandidate
from mapper.mapper import forcedCandidate
from model.locationTree import *
from model.locationParser import *
from xml.dom.minidom import parse, parseString
from xml.parsers.expat import ExpatError
import simplejson as json
import logging, logging.handlers, wukonghandler
import fnmatch
import shutil
from wkpfcomm import *
from xml2java.generator import Generator
from threading import Thread
from subprocess import Popen, PIPE, STDOUT
from collections import namedtuple
import distutils.dir_util
import mapper.mapper
from configuration import *
from globals import *

ChangeSets = namedtuple('ChangeSets', ['components', 'links', 'heartbeatgroups', 'deployIDs'])

class WuApplication:
  def __init__(self, id='', app_name='', desc='', file='', dir='', outputDir="", templateDir=TEMPLATE_DIR, componentXml=open(COMPONENTXML_PATH).read(),disabled=False):
    self.id = id
    self.app_name = app_name
    self.desc = desc
    self.file = file
    self.xml = ''
    self.dir = dir
    self.compiler = None
    self.version = 0
    self.returnCode = 1
    self.status = "" # deprecated, replaced by wukong_status and deploy_status
    self.deployed = False
    self.mapper = None
    self.inspector = None
    self.disabled = disabled
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

    self.changesets = ChangeSets([], [], [], [])

    # a log of mapping results warning or errors
    # format: a list of dict of {'msg': '', 'level': 'warn|error'}
    self.mapping_status = []

    # a log of deploying results warning or errors
    # format: a list of dict of {'msg': '', 'level': 'warn|error'}
    self.deploy_status = []
    self.deploy_ops = ''

    self.ftMonitorUnitLen = 3 # resilience = ftMonitorUnitLen - 1

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
    applicationEle = flowDom.getElementsByTagName('application')[0]
    self.name = applicationEle.getAttribute('name')
    self.disabled = len(applicationEle.getElementsByTagName('disabled')) > 0

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
    return {'id': self.id, 'app_name': self.app_name, 'desc': self.desc, 'dir': self.dir, 'xml': self.xml, 'version': self.version,'disabled':self.disabled}

  def __repr__(self):
    return json.dumps(self.config())

  def parseApplication(self):
      componentInstanceMap = {}
      wuLinkMap = {}
      applicationEle = self.applicationDom.getElementsByTagName('application')[0]
      application_hashed_name = applicationEle.getAttribute('name')
      self.disabled = len(applicationEle.getElementsByTagName('disabled')) > 0

      components = self.applicationDom.getElementsByTagName('component')
      self.instanceIds = []

      self.ft_group_instanceIds = {}

      # parse application XML to generate WuClasses, WuObjects and WuLinks
      for componentTag in components:
          # make sure application component is found in wuClassDef component list
          try:
              assert componentTag.getAttribute('type') in WuObjectFactory.wuclassdefsbyname.keys()
          except Exception as e:

            logging.error('unknown types for component found while parsing application')
            return #TODO: need to handle this

          fbp_type = componentTag.getAttribute('type')

          if componentTag.getElementsByTagName('location'):
            location = componentTag.getElementsByTagName('location')[0].getAttribute('requirement')
          else:
            location = '/'+LOCATION_ROOT

          if componentTag.getElementsByTagName('replica'):
            replica = int(componentTag.getElementsByTagName('replica')[0].getAttribute('requirement'))
          else:
            replica = 1

          if componentTag.getElementsByTagName('group_size'):
            group_size = int(componentTag.getElementsByTagName('group_size')[0].getAttribute('requirement'))
          else:
            group_size = 1

          if componentTag.getElementsByTagName('reaction_time'):
            reaction_time = float(componentTag.getElementsByTagName('reaction_time')[0].getAttribute('requirement'))
          else:
            reaction_time = 2.0

          if componentTag.getElementsByTagName('ft_group_size'):
            ft_group_size = int(componentTag.getElementsByTagName('ft_group_size')[0].getAttribute('requirement'))
          else:
            ft_group_size = 0

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
            component = WuComponent(index, location, group_size, replica, reaction_time, ft_group_size, fbp_type, application_hashed_name, properties)
            componentInstanceMap[componentTag.getAttribute('instanceId')] = component
            self.wuComponents[componentTag.getAttribute('instanceId')] = component
            self.changesets.components.append(component)
            self.instanceIds.append(int(index))

      # add server as component in node 0
      component = WuComponent(1, '/'+LOCATION_ROOT, 1, 1, 2.0, 0, 'Server', 0, {})
      componentInstanceMap[str(0)] = component
      self.wuComponents[0] = component
      self.changesets.components.append(component)
      self.instanceIds.append(0)
  
      duplicatedComponents = []
      ftComponents = []
      for componentInstance in self.changesets.components:
          componentInstanceId = componentInstance.index
          ft_group_size = componentInstance.ft_group_size
          # add duplicated components to attain FT capability
          new_instanceIds = []
          for i in range(1, ft_group_size):
              new_instanceIds.append(max(self.instanceIds) + 1)
              component = WuComponent(new_instanceIds[-1], componentInstance.location, componentInstance.group_size, componentInstance.replica, componentInstance.reaction_time, componentInstance.ft_group_size, componentInstance.type, componentInstance.application_hashed_name, componentInstance.properties) 
              componentInstanceMap[str(new_instanceIds[-1])] = component
              self.wuComponents[new_instanceIds[-1]] = component
              duplicatedComponents.append(component)
              self.instanceIds.append(new_instanceIds[-1])
          new_instanceIds.insert(0, componentInstanceId)
          self.ft_group_instanceIds[componentInstanceId] = new_instanceIds
          # add FT component for each duplicated component
          ftChain = []
          for i in range(ft_group_size):
              new_ft_instanceId = max(self.instanceIds) + 1
              to_instanceId = self.ft_group_instanceIds[componentInstanceId][i]
              ft_location = str(componentInstance.location.split('#')[0]) + '@' + str(to_instanceId) + '#'
              component = WuComponent(new_ft_instanceId, ft_location, componentInstance.group_size, componentInstance.replica, componentInstance.reaction_time, 0, 'FT', componentInstance.application_hashed_name, componentInstance.properties) 
              componentInstanceMap[str(new_ft_instanceId)] = component
              self.wuComponents[new_ft_instanceId] = component
              ftChain.append(component)
              self.instanceIds.append(new_ft_instanceId)
          ftComponents.append(ftChain)      

      for i in range(duplicatedComponents.__len__()):
          self.changesets.components.append(duplicatedComponents[i])
      for ftChain in ftComponents:
          for i in range(ftChain.__len__()):
              self.changesets.components.append(ftChain[i])

      #assumption: at most 99 properties for each instance, at most 999 instances
      #store hashed result of links to avoid duplicated links: (fromInstanceId*100+fromProperty)*100000+toInstanceId*100+toProperty
      linkSet = []
      # links

      for linkTag in self.applicationDom.getElementsByTagName('link'):
          from_component_id = linkTag.parentNode.getAttribute('instanceId')
          from_component = componentInstanceMap[from_component_id]
          from_property_name = linkTag.getAttribute('fromProperty').lower()
          from_property_id = WuObjectFactory.wuclassdefsbyname[from_component.type].properties[from_property_name].id
          from_component_ft_group_size = from_component.ft_group_size
          to_component_id = linkTag.getAttribute('toInstanceId')
          to_component = componentInstanceMap[to_component_id]
          to_property_name =  linkTag.getAttribute('toProperty').lower()
          to_property_id = WuObjectFactory.wuclassdefsbyname[to_component.type].properties[to_property_name].id
          to_component_ft_group_size = to_component.ft_group_size 

          # add link according to FBP app.xml
          if from_component_ft_group_size == 0 and to_component_ft_group_size == 0:
              hash_value = (int(from_component_id)*100+int(from_property_id))*100000+int(to_component_id)*100+int(to_property_id)
              if hash_value not in wuLinkMap.keys():
                  link = WuLink(from_component, from_property_name, to_component, to_property_name)
                  wuLinkMap[hash_value] = link
              self.changesets.links.append(wuLinkMap[hash_value])
          # add link for duplicated components
          elif from_component_ft_group_size > 0 and to_component_ft_group_size == 0:
              for duplicated_from_component_id in self.ft_group_instanceIds[from_component_id]:
                  from_component = componentInstanceMap[str(duplicated_from_component_id)]
                  hash_value = (int(duplicated_from_component_id)*100+int(from_property_id))*100000+int(to_component_id)*100+int(to_property_id)
                  if hash_value not in wuLinkMap.keys():
                      link = WuLink(from_component, from_property_name, to_component, to_property_name)
                      wuLinkMap[hash_value] = link
                  self.changesets.links.append(wuLinkMap[hash_value])
          elif from_component_ft_group_size == 0 and to_component_ft_group_size > 0:
              for duplicated_to_component_id in self.ft_group_instanceIds[to_component_id]:
                  to_component = componentInstanceMap[str(duplicated_to_component_id)]
                  hash_value = (int(from_component_id)*100+int(from_property_id))*100000+int(duplicated_to_component_id)*100+int(to_property_id)
                  if hash_value not in wuLinkMap.keys():
                      link = WuLink(from_component, from_property_name, to_component, to_property_name)
                      wuLinkMap[hash_value] = link
                  self.changesets.links.append(wuLinkMap[hash_value])
          elif from_component_ft_group_size > 0 and to_component_ft_group_size > 0:
              for duplicated_from_component_id in self.ft_group_instanceIds[from_component_id]:
                  from_component = componentInstanceMap[str(duplicated_from_component_id)]
                  for duplicated_to_component_id in self.ft_group_instanceIds[to_component_id]:
                      to_component = componentInstanceMap[str(duplicated_to_component_id)]
                      hash_value = (int(duplicated_from_component_id)*100+int(from_property_id))*100000+int(duplicated_to_component_id)*100+int(to_property_id)
                      if hash_value not in wuLinkMap.keys():
                          link = WuLink(from_component, from_property_name, to_component, to_property_name)
                          wuLinkMap[hash_value] = link
                      self.changesets.links.append(wuLinkMap[hash_value])

      #add monitoring related links
      if(MONITORING == 'true'):
          for instanceId, properties in self.monitorProperties.items():
              for name in properties:
                  hash_value = (int(instanceId)*100 + int(properties[name])*100000 + 0 + 0)
                  if hash_value not in wuLinkMap.keys():
                      link = WuLink(componentInstanceMap[str(instanceId)], name, componentInstanceMap[str(0)], 'input')
                      wuLinkMap[hash_value] = link
                  self.changesets.links.append(wuLinkMap[hash_value])

      # add link from the active property of ft components to the enable proerty of their designated components
      for ftChain in ftComponents:
          for ftComponent in ftChain:
              from_component = ftComponent
              from_component_id = from_component.index
              from_property_id = WuObjectFactory.wuclassdefsbyname[from_component.type].properties["active"].id
              to_component_id = from_component.location[from_component.location.find('@')+1:from_component.location.find('#')]
              to_component = componentInstanceMap[str(to_component_id)]
              to_property_id = WuObjectFactory.wuclassdefsbyname[to_component.type].properties["enable"].id
              hash_value = (int(from_component_id)*100+int(from_property_id))*100000+int(to_component_id)*100+int(to_property_id)
              if hash_value not in wuLinkMap.keys():
                  link = WuLink(from_component, 'active', to_component, 'enable')
                  wuLinkMap[hash_value] = link
              self.changesets.links.append(wuLinkMap[hash_value])
 
      # add link between ft components to form a monitoring chain
      ftMonitorUnitLen = self.ftMonitorUnitLen # this variable is used to change the number of resilience
      try: 
          assert (ftMonitorUnitLen >= 2)
      except Exception as e:
          logging.error('minimal ftMonitorUnitLen should be 2')
          ftMonitorUnitLen = 2
      ftGroupUnits = []
      for ftChain in ftComponents: 
          ftLen = len(ftChain)
          if ftLen < 2:
              continue
          for subUnitLen in range(2, ftMonitorUnitLen+1):
              ftGroupNum = ftLen - (subUnitLen -1)
              for i in range(ftGroupNum):
                  group = ftChain[i:i+subUnitLen]
                  ftGroupUnits.append(group)
      print "ftGroupUnits: ", ftGroupUnits
      for unit in ftGroupUnits:
          from_component = unit[0]
          from_component_id = from_component.index
          from_property_id = WuObjectFactory.wuclassdefsbyname[from_component.type].properties["hb"].id
          to_component = unit[-1]
          to_component_id = to_component.index
          toPropertyName = 'hb_' + 'g'*(len(unit)-2) + 'p_in'
          to_property_id = WuObjectFactory.wuclassdefsbyname[to_component.type].properties[toPropertyName].id 
          hash_value = (int(from_component_id)*100+int(from_property_id))*100000+int(to_component_id)*100+int(to_property_id)
          if hash_value not in wuLinkMap.keys():
              link = WuLink(from_component, 'hb', to_component, toPropertyName)
              wuLinkMap[hash_value] = link
          self.changesets.links.append(wuLinkMap[hash_value])

  def cleanAndCopyJava(self):
    # clean up the directory
    if os.path.exists(JAVA_OUTPUT_DIR):
      distutils.dir_util.remove_tree(JAVA_OUTPUT_DIR)

    os.mkdir(JAVA_OUTPUT_DIR)

    # copy WKDeployCustomComponents.xml to wkdeploy/java
    componentFile = os.path.join(self.dir, 'WKDeployCustomComponents.xml')
    if os.path.exists(componentFile):
      shutil.copy(componentFile, JAVA_OUTPUT_DIR)

      if not os.path.exists(os.path.join(JAVA_OUTPUT_DIR, 'WKDeployCustomComponents.xml')):
        self.errorDeployStatus("An error has encountered while copying WKDeployCustomComponents.xml to java dir in wkdeploy!")

    # copy java implementation to wkdeploy/java
    # recursive scan
    for root, dirnames, filenames in os.walk(self.dir):
      for filename in fnmatch.filter(filenames, "*.java"):
        javaFile = os.path.join(root, filename)
        shutil.copy(javaFile, JAVA_OUTPUT_DIR)

        if not os.path.exists(os.path.join(JAVA_OUTPUT_DIR, filename)):
          self.errorDeployStatus("An error has encountered while copying %s to java dir in wkdeploy!" % (filename))


  def generateJava(self):
      mapper.mapper.dump_changesets(self.changesets)
      Generator.generate(self.name, self.changesets)

  def mapping(self, locTree, routingTable, predicts=[], mapFunc=mapper.mapper.forcedCandidate):
      #input: nodes, WuObjects, WuLinks, WuClassDefs
      #output: assign node id to WuObjects
      # TODO: mapping results for generating the appropriate instiantiation for different nodes

      return mapFunc(self, self.changesets, routingTable, locTree, predicts)

  def map(self, location_tree, routingTable):
    self.changesets = ChangeSets([], [], [], [])
    self.parseApplication()
    result = self.mapping(location_tree, routingTable)
    logging.info("Mapping Results")
    logging.info(self.changesets)
    return result

  def del_and_remap(self, location_tree, routingTable):
    self.changesets = ChangeSets([], [], [], [])
    self.parseApplication()
    result = mapper.mapper.delete_application(self, self.changesets, routingTable, location_tree)
    logging.info("Mapping Results")
    logging.info(self.changesets)
    return result



  def deploy_with_discovery(self,*args):
    #node_ids = [info.id for info in getComm().getActiveNodeInfos(force=False)]
    #node_ids = set([x.wunode.id for component in self.changesets.components for x in component.instances])
    node_ids = set([x for x in self.changesets.deployIDs])
    print node_ids
    res = self.deploy(node_ids,*args)
    return res

  def deploy(self, destination_ids, platforms):
    master_busy()
    app_path = self.dir
    self.clearDeployStatus()

    for platform in platforms:
      platform_dir = os.path.join(app_path, platform)

      self.logDeployStatus("Preparing java library code...")
      gevent.sleep(0)

      try:
        self.cleanAndCopyJava()
      except Exception as e:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        traceback.print_exception(exc_type, exc_value, exc_traceback,
                                      limit=2, file=sys.stdout)
        self.errorDeployStatus("An error has encountered while cleaning and copying java files to java dir in wkdeploy! Backtrace is shown below:")
        self.errorDeployStatus(exc_traceback)
        return False

      self.logDeployStatus("Generating java application...")

      try:
        self.generateJava()
      except Exception as e:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        traceback.print_exception(exc_type, exc_value, exc_traceback,
                                      limit=9, file=sys.stdout)
        self.errorDeployStatus("An error has encountered while generating java application! Backtrace is shown below:")
        self.errorDeployStatus(exc_traceback)
        return False
      gevent.sleep(0)

      # Build the Java code
      self.logDeployStatus('Compressing application code to bytecode format...')
      pp = Popen('cd %s/src; gradle -PdjConfigname=wkdeploy createAppArchive' % (ROOT_PATH), shell=True, stdout=PIPE, stderr=PIPE)
      self.returnCode = None
      (infomsg,errmsg) = pp.communicate()
      gevent.sleep(0)

      self.version += 1
      if pp.returncode != 0:
        self.logDeployStatus(infomsg)
        self.errorDeployStatus('Error generating wkdeploy.dja! Backtrack is shown below:')
        self.errorDeployStatus(errmsg)
        return False
      self.logDeployStatus('Compression finished')
      gevent.sleep(0)

      comm = getComm()

      # Deploy nvmdefault.h to nodes
      self.logDeployStatus('Preparing to deploy to nodes %s' % (str(destination_ids)))
      remaining_ids = copy.deepcopy(destination_ids)
      gevent.sleep(0)

      # Remove Server node ID?
      try:
          destination_ids.remove(WUKONG_GATEWAY)
          remaining_ids.remove(WUKONG_GATEWAY)
      except:
          pass
      for node_id in destination_ids:
        node = WuNode.node_dict[node_id]
        print "Deploy to node %d type %s"% (node_id, node.type)
        if node.type == 'native': #We need to review the logic here ---- Sen
          continue
        remaining_ids.remove(node_id)
        self.logDeployStatus("Deploying to node %d, remaining %s" % (node_id, str(remaining_ids)))
        retries = 3
        if not comm.reprogram(node_id, os.path.join(ROOT_PATH, 'src', 'build', 'wkdeploy', 'app_infusion', 'app_infusion.dja'), retry=retries):
          self.errorDeployStatus("Deploy was unsucessful after %d tries!" % (retries))
          return False
        self.logDeployStatus('...has completed')
    self.logDeployStatus('Application has been deployed!')

    #save map result
    if os.path.isfile("changesets.pkl"):
      os.remove("changesets.pkl")
    if os.path.isfile("shared_links.pkl"):
      os.remove("shared_links.pkl")
    os.rename("changesets.tmp", "changesets.pkl")
    if os.path.isfile("shared_links.tmp"):
      os.rename("shared_links.tmp", "shared_links.pkl")

    self.stopDeployStatus()
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
    if self.map(location_tree, routingTable):
      self.deploy([info.id for info in node_infos], DEPLOY_PLATFORMS)
    master_available()
