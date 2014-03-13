# vim: sw=2 ts=2 expandtab
import sys, os, traceback, copy
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
from parser import *
from locationTree import *
from xml.dom.minidom import parse, parseString
from xml.parsers.expat import ExpatError
import simplejson as json
import logging, logging.handlers, wukonghandler
from collections import namedtuple
from locationParser import *
#from codegen import CodeGen
from xml2java.generator import Generator
import copy
from threading import Thread
import traceback
import time
import re
import StringIO
import shutil, errno
import datetime
from subprocess import Popen, PIPE, STDOUT

from configuration import *
from globals import *

# allcandidates are all node ids ([int])
def constructHeartbeatGroups(heartbeatgroups, routingTable, allcandidates):
  del heartbeatgroups[:]

  while len(allcandidates) > 0:
    heartbeatgroup = namedtuple('heartbeatgroup', ['nodes', 'period'])
    heartbeatgroup.nodes = []
    heartbeatgroup.period = 1
    if len(heartbeatgroup.nodes) == 0:
      heartbeatgroup.nodes.append(allcandidates.pop(0)) # should be random?
      pivot = heartbeatgroup.nodes[0]
      if pivot.id in routingTable:
        for neighbor in routingTable[pivot.id]:
          neighbor = int(neighbor)
          if neighbor in [x.id for x in allcandidates]:
            for candidate in allcandidates:
              if candidate.id == neighbor:
                heartbeatgroup.nodes.append(candidate)
                allcandidates.remove(candidate)
                break
      heartbeatgroups.append(heartbeatgroup)

# assign periods
def determinePeriodForHeartbeatGroups(components, heartbeatgroups):
  for component in components:
    for wuobject in component.instances:
      for group in heartbeatgroups:
        if wuobject.node_id in [x.id for x in group.nodes]:
          #group heartbeat is reactiontime divided by 2, then multiplied by 1000 to microseconds
          newperiod = int(float(component.reaction_time) / 2.0 * 1000.0)
          if not group.period or (group.period and group.period > newperiod):
            group.period = newperiod
          break

def sortCandidates(wuObjects):
    nodeScores = {}
    for candidates in wuObjects:
      for node in candidates:
        if node[0] in nodeScores:
          nodeScores[node[0]] += 1
        else:
          nodeScores[node[0]] = 1

    for candidates in wuObjects:
      sorted(candidates, key=lambda node: nodeScores[node[0]], reverse=True)

##########changeset example #######
#ChangeSets(components=[
#    WuComponent(
#      {'index': 0, 'reaction_time': 1.0, 'group_size': 1, 'application_hashed_name': u'f92ea1839dc16d7396db358365da7066', 'heartbeatgroups': [], 'instances': [
#    WuObject(
#      {'node_identity': 1, 'wuproperty_cache': [], 'wuclassdef_identity': 11, 'virtual': 0, 'port_number': 0, 'identity': 1}
#    )], 'location': 'WuKong', 'properties': {}, 'type': u'Light_Sensor'}
#    )], links=[], heartbeatgroups=[])
#############################

def firstCandidate(logger, changesets, routingTable, locTree):
    set_wukong_status('Mapping')
    logger.clearMappingStatus() # clear previous mapping status

    #input: nodes, WuObjects, WuLinks, WuClassDefsm, wuObjects is a list of wuobject list corresponding to group mapping
    #output: assign node id to WuObjects
    # TODO: mapping results for generating the appropriate instiantiation for different nodes


    #clear all "mapped" tags in every node before mapping
    for nodeid in WuNode.node_dict:
        node = locTree.getNodeInfoById(nodeid)
        if node != None:
            for wuobj in node.wuobjects.values():
                wuobj.mapped = False
            
    # construct and filter candidates for every component on the FBP (could be the same wuclass but with different policy)
    for component in changesets.components:
        # filter by location
        locParser = LocationParser(locTree)
        print component
        try:
            candidates, rating = locParser.parse(component.location)
            print "candidates", candidates, rating
        except:
            #no mapping result
            exc_type, exc_value, exc_traceback = sys.exc_info()
            msg = 'Cannot find match for location query "'+ component.location+'" of wuclass "'+ component.type+ '".' 
            logger.errorMappingStatus(msg)
            set_wukong_status(msg)
            return False
            #candidates = locTree.root.getAllAliveNodeIds()


        if len(candidates) < component.group_size:
            msg = 'There is not enough candidates %r for component %s, but mapper will continue to map' % (candidates, component)
            set_wukong_status(msg)
            logger.warnMappingStatus(msg)
                
        # construct wuobjects, instances of component
        for candidate in candidates:
            wuclassdef = WuObjectFactory.wuclassdefsbyname[component.type]
            node = locTree.getNodeInfoById(candidate)
            available_wuobjects = [wuobject for wuobject in node.wuobjects.values() if wuobject.wuclassdef.id == wuclassdef.id]
            
            has_wuclass = wuclassdef.id in node.wuclasses.keys()
            wuobj_found = False
            for avail_wuobj in available_wuobjects:
                # use existing native wuobject, caution given to obj mapped due to previous candidates
                if not avail_wuobj.virtual and not avail_wuobj.mapped:
                    print "appending native for", node.id, avail_wuobj.wunode.location
                    component.instances.append(avail_wuobj)
                    avail_wuobj.mapped = True
                    wubj_found = True
                    break
                    
            if has_wuclass and (not wuobj_found):    # create a new wuobject from existing wuclasses published from node
                sensorNode = locTree.sensor_dict[node.id]
                sensorNode.initPortList(forceInit = False)
                port_number = sensorNode.reserveNextPort()
                wuobject = WuObjectFactory.createWuObject(wuclassdef, node, port_number,True)
                print "appending vitual for", node.id
                component.instances.append(wuobject)
                  
            elif (not wuobj_found) and node.type != 'native' and node.type != 'picokong' and wuclassdef.virtual==True:
                # create a new virtual wuobject where the node 
                # doesn't have the wuclass for it
                sensorNode = locTree.sensor_dict[node.id]
                sensorNode.initPortList(forceInit = False)
                port_number = sensorNode.reserveNextPort()
                wuobject = WuObjectFactory.createWuObject(wuclassdef, node, port_number, True)
                component.instances.append(wuobject)

        print ([inst.wunode.id for inst in component.instances])
        #this is ignoring ordering of policies, eg. location policy, should be fixed or replaced by other algorithm later--- Sen
        component.instances = sorted(component.instances, key=lambda wuObject: wuObject.virtual, reverse=False)
        # limit to min candidate if possible
        # here is a bug if there are not enough elements in instances list   ---Sen
        component.instances = component.instances[:component.group_size]
        print ([inst.wunode.id for inst in component.instances])
        for inst in component.instances[component.group_size:]:     #roll back unused virtual wuclasses created in previous step
          if inst.virtual:
            WuObjectFactory.remove(inst.wunode, inst.port_number)
        print ([inst.wunode.id for inst in component.instances])
        if len(component.instances) == 0:
          logger.errorMappingStatus('No avilable match could be found for component %s' % (component))
          return False

    # Done looping components

    # sort candidates
    # TODO:simple sorting, first fit, last fit, hardware fit, etc
    #sortCandidates(changesets.components)

    # TODO: will uncomment this once I port my stuff from NanoKong v1
    # construct heartbeat groups plus period assignment
    #allcandidates = set()
    #for component in changesets.components:
        #for wuobject in component.instances:
            #allcandidates.add(wuobject.wunode().id)
    #allcandidates = list(allcandidates)
    #allcandidates = map(lambda x: WuNode.find(id=x), allcandidates)
    # constructHeartbeatGroups(changesets.heartbeatgroups, routingTable, allcandidates)
    # determinePeriodForHeartbeatGroups(changesets.components, changesets.heartbeatgroups)
    #logging.info('heartbeatGroups constructed, periods assigned')
    #logging.info(changesets.heartbeatgroups)

    #Deprecated: tree will be rebuild before mapping
    #delete and roll back all reservation during mapping after mapping is done, next mapping will overwritten the current one
    #for component in changesets.components:
        #for wuobj in component.instances:
            #senNd = locTree.getSensorById(wuobj.wunode().id)
            #for j in senNd.temp_port_list:
                #senNd.port_list.remove(j)
            #senNd.temp_port_list = []

    set_wukong_status('')
    return True
