"""
It manages the global set of applications, and the life cycle of an application.
The life cycle includes stages of below:

1) Load from XML
2) Clean and Copy Packaging Folder
3) Code Generation
4) map
5) deploy
6) reconfiguration
"""
import sys, os, traceback, time, re, copy
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
from wkpf.model.models import WuClassDef, WuComponent, WuLink
import fnmatch
import shutil
from wkpf.wkpfcomm import *
from wkpf.xml2java.generator import Generator
from subprocess import Popen, PIPE, STDOUT
from collections import namedtuple
import distutils.dir_util
from wkpf.model.models import WuApplication
from configuration import *
from mapper.mapper import *
import shutil, errno
from wkpf.util import *
import logging

ChangeSets = namedtuple('ChangeSets', ['components', 'links', 'heartbeatgroups'])

class ApplicationManager:
    __application_manager = None

    @classmethod
    def init(cls):
        if(cls.__application_manager == None):
            cls.__application_manager = ApplicationManager()

        return cls.__application_manager

    def __init__(self):
        self._active_ind = None
        self._applications = []
        self._app_map = {}  # id to application
        self.updateApplications()

    def hasApplication(self, app_id):
        return app_id in self._app_map

    def createApplication(self, app_id, app_name):

        # copy base for the new application
        logging.info('creating application... "%s"' % (app_name))
        copyAnything(BASE_DIR, os.path.join(APP_DIR, app_id))

        app = WuApplication(id=app_id, app_name=app_name, dir=os.path.join(APP_DIR, app_id))
        logging.info('app constructor')
        logging.info(app.app_name)
        self._applications.append(app)
        self._app_map[app_id] = app

        # dump config file to app
        logging.info('saving application configuration...')
        saveConfig(app)
        return app

    def renameApplication(self, app_id, app_name):
         self._app_map[app_id].app_name
         saveConfig(_app_map[app_id])

    def getAppIndex(self, app_id):
        # make sure it is not unicode
        app_id = app_id.encode('ascii','ignore')
        for index, app in enumerate(self._applications):
            if app.id == app_id:
                return index
        return None

    def getAllApplications(self):
        return self._applications

    def getApplication(self, id):
        if id in self._app_map:
            return self._app_map[id]
        else:
            return None

    def addApplication(self, app):
        self._applications.append(app);
        self._app_map[app.id] = app;

    def deleteApplication(self, i):
        try:
            shutil.rmtree(self._applications[i].dir)
            del self._app_map[_applications[i].id]
            self._applications.pop(i)
            return True
        except Exception as e:
            exc_type, exc_value, exc_traceback = sys.exc_info()
            print traceback.print_exception(exc_type, exc_value, exc_traceback, limit=2, file=sys.stdout)
        return False

    def loadAppFromDir(self, dir):
        app = WuApplication(dir=dir)
        self.loadConfig(app)
        return app

    def updateXML(self, app, xml):
            self.xml = xml
            self.setFlowDom(parseString(self.xml))
            self.saveConfig()
            f = open(os.path.join(self.dir, self.id + '.xml'), 'w')
            f.write(xml)
            f.close()

    def loadConfig(self, app):
        config = json.load(open(os.path.join(app.dir, 'config.json')))
        app.id = config['id']
        try:
            app.app_name = config['app_name']
        except:
            app.app_name='noname';
        app.desc = config['desc']
        # self.dir = config['dir']
        app.xml = config['xml']
        try:
            dom = parseString(app.xml)
            app.setFlowDom(dom)
        except ExpatError:
            pass

    def saveConfig(self, app):
        json.dump(app.config(), open(os.path.join(app.dir, 'config.json'), 'w'))

    def updateApplications(self):
        logging.info('updating applications:')
        application_basenames = [os.path.basename(app.dir) for app in self._applications]

        for dirname in os.listdir(APP_DIR):
            app_dir = os.path.join(APP_DIR, dirname)
            if dirname.lower() == 'base': continue
            if not os.path.isdir(app_dir): continue

            logging.info('scanning %s:' % (dirname))
            if dirname not in application_basenames:
                logging.info('%s' % (dirname))
                app = self.loadAppFromDir(app_dir)
                self._applications.append(app)
                self._app_map[app.id] = app
                application_basenames = [os.path.basename(app.dir) for app in self._applications]

    # Build up internal data structure from xml
    def parseApplication(self, app):
        componentInstanceMap = {}
        wuLinkMap = {}
        application_hashed_name = app.applicationDom.getElementsByTagName('application')[0].getAttribute('name')

        components = app.applicationDom.getElementsByTagName('component')
        app.instanceIds = []

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
            app.monitorProperties[index] = {}

            # set monitoring properties index for components in application
            for propertyTag in componentTag.getElementsByTagName('monitorProperty'):
                for attr in propertyTag.attributes.values():
                    app.monitorProperties[index][attr.name] = attr.value.strip()

            if index in app.instanceIds:

                #wucomponent already appears in other pages, merge property requirement, suppose location etc are the same
                app.wuComponents[index].properties = dict(app.wuComponents[index].properties.items() + properties.items())
            else:
                component = WuComponent(index, location, group_size, reaction_time, type, application_hashed_name, properties)
                componentInstanceMap[componentTag.getAttribute('instanceId')] = component
                app.wuComponents[componentTag.getAttribute('instanceId')] = component
                app.changesets.components.append(component)
                app.instanceIds.append(index)

        # add server as component in node 0
        component = WuComponent(1, '/'+LOCATION_ROOT, 1, 2.0, 'Server', 0, {})
        componentInstanceMap[0] = component
        app.wuComponents[0] = component
        app.changesets.components.append(component)
        app.instanceIds.append(0)
        #assumption: at most 99 properties for each instance, at most 999 instances
        #store hashed result of links to avoid duplicated links: (fromInstanceId*100+fromProperty)*100000+toInstanceId*100+toProperty
        linkSet = []
        # links
        for linkTag in app.applicationDom.getElementsByTagName('link'):
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
            app.changesets.links.append(wuLinkMap[hash_value])

        #add monitoring related links
        if(MONITORING == 'true'):
            for instanceId, properties in self.monitorProperties.items():
                for name in properties:
                    hash_value = (int(instanceId)*100 + int(properties[name])*100000 + 0 + 0)
                    if hash_value not in wuLinkMap.keys():
                        link = WuLink(componentInstanceMap[instanceId], name, componentInstanceMap[0], 'input')
                        wuLinkMap[hash_value] = link
                    app.changesets.links.append(wuLinkMap[hash_value])

    def getActiveApplication(self):
        try:
            return self._applications[self._active_ind]
        except:
            return None

    def setActiveApplicationIndex(self, app_id):
        self._active_id = self.getAppIndex(app_id)

    def updateApplication(app_id, name, desc):
         self._app_map[app_ind].app_name = name
         self._app_map[app_ind].desc = desc
         self._app_map[app_ind].saveConfig()

    def cleanAndCopyJava(self, app):
        # clean up the directory
        if os.path.exists(JAVA_OUTPUT_DIR):
            distutils.dir_util.remove_tree(JAVA_OUTPUT_DIR)
            s.mkdir(JAVA_OUTPUT_DIR)

        # copy WKDeployCustomComponents.xml to wkdeploy/java
        componentFile = os.path.join(wuapplication.dir, 'WKDeployCustomComponents.xml')
        if os.path.exists(componentFile):
            shutil.copy(componentFile, JAVA_OUTPUT_DIR)

        if not os.path.exists(os.path.join(JAVA_OUTPUT_DIR, 'WKDeployCustomComponents.xml')):
            app.errorDeployStatus("An error has encountered while copying WKDeployCustomComponents.xml to java dir in wkdeploy!")

        # copy java implementation to wkdeploy/java
        # recursive scan
        for root, dirnames, filenames in os.walk(self.dir):
            for filename in fnmatch.filter(filenames, "*.java"):
                javaFile = os.path.join(root, filename)
                shutil.copy(javaFile, JAVA_OUTPUT_DIR)

            if not os.path.exists(os.path.join(JAVA_OUTPUT_DIR, filename)):
                app.errorDeployStatus("An error has encountered while copying %s to java dir in wkdeploy!" % (filename))

    def generateJava(self, wuapplication):
        Generator.generate(wuapplication.name, wuapplication.changesets)

    def map(self, app, location_tree, routingTable, mapFunc=firstCandidate):
        app.changesets = ChangeSets([], [], [])
        self.parseApplication(app)
        result = mapFunc(app, app.changesets, routingTable, location_tree)
        logging.info("Mapping Results")
        logging.info(app.changesets)
        return result

    def deploy_with_discovery(self, app, *args):
        node_ids = set([x.wunode.id for component in app.changesets.components for x in component.instances])
        self.deploy(node_ids, *args)

    def deploy(self, destination_ids, platforms):
        sysmanager = SystemManager.init()
        sysmanager.setMasterBusy();
        wuapplication = getActiveApplication()
        app_path = wuapplication.dir
        wuapplication.clearDeployStatus()

        for platform in platforms:
            platform_dir = os.path.join(app_path, platform)

            wuapplication.logDeployStatus("Preparing java library code...")
            gevent.sleep(0)

            try:
                cleanAndCopyJava(wuapplication)
            except Exception as e:
                exc_type, exc_value, exc_traceback = sys.exc_info()
                traceback.print_exception(exc_type, exc_value, exc_traceback, limit=2, file=sys.stdout)
                wuapplication.errorDeployStatus("An error has encountered while cleaning and copying java files to java dir in wkdeploy! Backtrace is shown below:")
                wuapplication.errorDeployStatus(exc_traceback)
                return False

            self.logDeployStatus("Generating java application...")

            try:
                self.generateJava(wuapplication)
            except Exception as e:
                exc_type, exc_value, exc_traceback = sys.exc_info()
                traceback.print_exception(exc_type, exc_value, exc_traceback, limit=9, file=sys.stdout)
                wuapplication.errorDeployStatus("An error has encountered while generating java application! Backtrace is shown below:")
                wuapplication.errorDeployStatus(exc_traceback)
                return False
            gevent.sleep(0)

            # Build the Java code
            wuapplication.logDeployStatus('Compressing application code to bytecode format...')
            pp = Popen('cd %s/..; ant clean; ant' % (JAVA_OUTPUT_DIR), shell=True, stdout=PIPE, stderr=PIPE)
            wuapplication.returnCode = None
            (infomsg,errmsg) = pp.communicate()
            gevent.sleep(0)

            wuapplication.version += 1
            if pp.returncode != 0:
                wuapplication.logDeployStatus(infomsg)
                wuapplication.errorDeployStatus('Error generating wkdeploy.dja! Backtrack is shown below:')
                wuapplication.errorDeployStatus(errmsg)
                return False
            wuapplication.logDeployStatus('Compression finished')
            gevent.sleep(0)

            comm = getComm()

            # Deploy nvmdefault.h to nodes
            wuapplication.logDeployStatus('Preparing to deploy to nodes %s' % (str(destination_ids)))
            remaining_ids = copy.deepcopy(destination_ids)
            gevent.sleep(0)

            destination_ids.remove(1)
            remaining_ids.remove(1)

            for node_id in destination_ids:
                node = WuNode.node_dict[node_id]
                print "Deploy to node %d type %s"% (node_id, node.type)
                if node.type == 'native': #We need to review the logic here ---- Sen
                    continue
                remaining_ids.remove(node_id)
                wuapplication.logDeployStatus("Deploying to node %d, remaining %s" % (node_id, str(remaining_ids)))
                retries = 3
                if not comm.reprogram(node_id, os.path.join(JAVA_OUTPUT_DIR, '..', 'build', 'wkdeploy.dja'), retry=retries):
                    wuapplication.errorDeployStatus("Deploy was unsucessful after %d tries!" % (retries))
                    return False
            wuapplication.logDeployStatus('...has completed')

        wuapplication.logDeployStatus('Application has been deployed!')
        wuapplication.stopDeployStatus()
        sysmanager.setMasterAvailable()
        return True

    def reconfiguration(self):

        global location_tree
        global routingTable
        sysmanager = SystemManager.init()
        sysmanager.setMasterBusy();
        wuapplication = getActiveApplication()
        wuapplication.status = "Start reconfiguration"
        node_infos = getComm().getActiveNodeInfos(force=True)
        location_tree = LocationTree(LOCATION_ROOT)
        location_tree.buildTree(node_infos)
        routingTable = getComm().getRoutingInformation()
        if self.map(location_tree, routingTable):
            self.deploy([info.id for info in node_infos], DEPLOY_PLATFORMS)
        sysmanager.setMasterAvailable()


