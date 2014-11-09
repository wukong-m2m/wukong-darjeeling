“”“
It manages the global set of applications, and the life cycle of an application.
The life cycle includes stages of below:

1) Load from XML
2) Clean and Copy Packaging Folder
3) Code Generation
4) map
5) deploy
6) reconfiguration
”“”

# vim:ts=2 sw=2 expandtab
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
import sys, os, traceback, time, re, copy
from wkpf.model.models import WuClassDef, WuComponent, WuLink
import fnmatch
import shutil
from wkpfcomm import *
from xml2java.generator import Generator
from subprocess import Popen, PIPE, STDOUT
from collections import namedtuple
import distutils.dir_util
from configuration import *
from globals import *


class ApplicationManager:
    __application_manager = None

    @classmethod
    def init(cls):
        if(cls.__application_manager = None):
            cls.__application_manager = ApplicationManager([], False)

         return cls.__application_manager

    def __init__(self, apps, status):
        self._busy = status
        self._active_ind = None
        self._applications = apps
        self._app_map = {}

    def master_busy(self):
        self._busy = True

    def master_available(self):
        self._busy = False

    def hasApplication(self, app_id):
        return app_id in self._app_map

    def createApplication(self, app_id):

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
        app.saveConfig()
        return app

    def renameApplication(self, app_id, app_name):
         self._app_map[app_id].app_name
         self._app_map[app_id].saveConfig()

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
            def self._app_map[_applications[i].id]
            self._applications.pop(i)
            return True
        except Exception as e:
            exc_type, exc_value, exc_traceback = sys.exc_info()
            print traceback.print_exception(exc_type, exc_value, exc_traceback, limit=2, file=sys.stdout)
        return False

    def loadAppFromDir(self, dir):
        app = WuApplication(dir=dir)
        app.loadConfig()
        return app

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
                app = loadAppFromDir(app_dir)
                self._applications.append(app)
                self._app_map[app.id] = app
                application_basenames = [os.path.basename(app.dir) for app in self._applications]

    def getActiveApplication(self):
        try:
            return self._applications[self._active_ind]
        except:
            return None

    def setActiveApplicationIndex(self, app_id):
        self._active_id = getAppIndex(app_id)

    def updateApplication(app_id, name, desc):
         self._app_map[app_ind].app_name = name
         self._app_map[app_ind].desc = desc
         self._app_map[app_ind].saveConfig()

    def cleanAndCopyJava(self, wuapplication):
        # clean up the directory
        if os.path.exists(JAVA_OUTPUT_DIR):
            distutils.dir_util.remove_tree(JAVA_OUTPUT_DIR)
            s.mkdir(JAVA_OUTPUT_DIR)

        # copy WKDeployCustomComponents.xml to wkdeploy/java
        componentFile = os.path.join(wuapplication.dir, 'WKDeployCustomComponents.xml')
        if os.path.exists(componentFile):
            shutil.copy(componentFile, JAVA_OUTPUT_DIR)

        if not os.path.exists(os.path.join(JAVA_OUTPUT_DIR, 'WKDeployCustomComponents.xml')):
            wuapplication.errorDeployStatus("An error has encountered while copying WKDeployCustomComponents.xml to java dir in wkdeploy!")

        # copy java implementation to wkdeploy/java
        # recursive scan
        for root, dirnames, filenames in os.walk(self.dir):
            for filename in fnmatch.filter(filenames, "*.java"):
                javaFile = os.path.join(root, filename)
                shutil.copy(javaFile, JAVA_OUTPUT_DIR)

            if not os.path.exists(os.path.join(JAVA_OUTPUT_DIR, filename)):
                wuapplication.errorDeployStatus("An error has encountered while copying %s to java dir in wkdeploy!" % (filename))

    def generateJava(self, wuapplication):
        Generator.generate(wuapplication.name, wuapplication.changesets)

    def map(self, location_tree, routingTable, mapFunc=firstCandidate):
        wuapplication = getActiveApplication()
        wuapplication.changesets = ChangeSets([], [], [])
        wuapplication.parseApplication()
        result = mapFunc(wuapplication, wuapplication.changesets, routingTable, locTree)
        logging.info("Mapping Results")
        logging.info(self.changesets)
        return result

    def deploy_with_discovery(self, *args):
        node_ids = set([x.wunode.id for component in wuapplication.changesets.components for x in component.instances])
        self.deploy(node_ids, *args)

    def deploy(self, destination_ids, platforms):
        master_busy()
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
        master_available()
        return True

    def reconfiguration(self):

        global location_tree
        global routingTable
        master_busy()
        wuapplication = getActiveApplication()
        wuapplication.status = "Start reconfiguration"
        node_infos = getComm().getActiveNodeInfos(force=True)
        location_tree = LocationTree(LOCATION_ROOT)
        location_tree.buildTree(node_infos)
        routingTable = getComm().getRoutingInformation()
        if self.map(location_tree, routingTable):
            self.deploy([info.id for info in node_infos], DEPLOY_PLATFORMS)
        master_available()


