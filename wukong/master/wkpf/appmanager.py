# vim:ts=2 sw=2 expandtab
import sys, os, traceback, time, re, copy
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
from model.models import WuClassDef, WuComponent, WuLink
import fnmatch
import shutil
from wkpfcomm import *
from xml2java.generator import Generator
from subprocess import Popen, PIPE, STDOUT
from collections import namedtuple
import distutils.dir_util
from configuration import *
from globals import *

# A class that manages the whole life cycle of an application. The life cycle includes stages of below:
# 1) Load from XML
# 2) Clean and Copy Packaging Folder
# 3) Code Generation
# 4) map
# 5) deploy
# 6) reconfiguration

class ApplicationManager:

    @staticmethod
    def cleanAndCopyJava(wuapplication):
      # clean up the directory
      if os.path.exists(JAVA_OUTPUT_DIR):
        distutils.dir_util.remove_tree(JAVA_OUTPUT_DIR)

        s.mkdir(JAVA_OUTPUT_DIR)

      # copy WKDeployCustomComponents.xml to wkdeploy/java
      componentFile = os.path.join(self.dir, 'WKDeployCustomComponents.xml')
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

    @staticmethod
    def generateJava(wuapplication):
      Generator.generate(wuapplication.name, wuapplication.changesets)

    @staticmethod
    def map(wuapplication, location_tree, routingTable, mapFunc=firstCandidate):
      wuapplication.changesets = ChangeSets([], [], [])
      wuapplication.parseApplication()
      result = mapFunc(wuapplication, wuapplication.changesets, routingTable, locTree)
      logging.info("Mapping Results")
      logging.info(self.changesets)
      return result

    @staticmethod
    def deploy_with_discovery(wuapplication, *args):
      #node_ids = [info.id for info in getComm().getActiveNodeInfos(force=False)]
      node_ids = set([x.wunode.id for component in wuapplication.changesets.components for x in component.instances])
      self.deploy(node_ids,*args)

    @staticmethod
    def deploy(wuapplication, destination_ids, platforms):
      master_busy()
      app_path = self.dir
      wuapplication.clearDeployStatus()

      for platform in platforms:
        platform_dir = os.path.join(app_path, platform)

        wuapplication.logDeployStatus("Preparing java library code...")
        gevent.sleep(0)

        try:
          wuapplication.cleanAndCopyJava()
        except Exception as e:
          exc_type, exc_value, exc_traceback = sys.exc_info()
          traceback.print_exception(exc_type, exc_value, exc_traceback, limit=2, file=sys.stdout)
          wuapplication.errorDeployStatus("An error has encountered while cleaning and copying java files to java dir in wkdeploy! Backtrace is shown below:")
          wuapplication.errorDeployStatus(exc_traceback)
          return False

         self.logDeployStatus("Generating java application...")

        try:
          self.generateJava()
        except Exception as e:
          exc_type, exc_value, exc_traceback = sys.exc_info()
          traceback.print_exception(exc_type, exc_value, exc_traceback, limit=9, file=sys.stdout)
          wuapplication.errorDeployStatus("An error has encountered while generating java application! Backtrace is shown below:")
          wuapplication.errorDeployStatus(exc_traceback)
          return False
        gevent.sleep(0)

        # Build the Java code
        self.logDeployStatus('Compressing application code to bytecode format...')
        pp = Popen('cd %s/..; ant clean; ant' % (JAVA_OUTPUT_DIR), shell=True, stdout=PIPE, stderr=PIPE)
        self.returnCode = None
        (infomsg,errmsg) = pp.communicate()
        gevent.sleep(0)

        self.version += 1
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

    @staticmethod
    def reconfiguration(wuapplication):
      global location_tree
      global routingTable
      master_busy()
      application.status = "Start reconfiguration"
      node_infos = getComm().getActiveNodeInfos(force=True)
      location_tree = LocationTree(LOCATION_ROOT)
      location_tree.buildTree(node_infos)
      routingTable = getComm().getRoutingInformation()
      if self.map(location_tree, routingTable):
        self.deploy([info.id for info in node_infos], DEPLOY_PLATFORMS)
      master_available()