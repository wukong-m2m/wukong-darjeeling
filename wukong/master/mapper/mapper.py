# vim: sw=2 ts=2 expandtab
import sys, os, traceback, copy
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
from parser import *
from wkpf.model.locationTree import *
from xml.dom.minidom import parse, parseString
from xml.parsers.expat import ExpatError
import simplejson as json
import logging, logging.handlers, wkpf.wukonghandler
from collections import namedtuple
from wkpf.model.locationParser import *
#from codegen import CodeGen
from wkpf.xml2java.generator import Generator
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
from wkpf.globals import *
import pickle

ChangeSets = namedtuple('ChangeSets', ['components', 'links', 'heartbeatgroups'])

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

def firstCandidate(logger, changesets, routingTable, locTree, flag = None):
    set_wukong_status('Mapping')
    logger.clearMappingStatus() # clear previous mapping status

    #input: nodes, WuObjects, WuLinks, WuClassDefsm, wuObjects is a list of wuobject list corresponding to group mapping
    #output: assign node id to WuObjects
    # TODO: mapping results for generating the appropriate instiantiation for different nodes
    mapping_result = True
    #clear all "mapped" tags in every node before mapping
    if flag == None:
        for nodeid in WuNode.node_dict:
            node = locTree.getNodeInfoById(nodeid)
            if node != None:
                for wuobj in node.wuobjects.values():
                    wuobj.mapped = False

    # construct and filter candidates for every component on the FBP (could be the same wuclass but with different policy)
    for component in changesets.components:
        # filter by location
        locParser = LocationParser(locTree)
        msg = ''
        print component
        try:
            candidates, rating = locParser.parse(component.location)
            print "candidates: ", candidates, "rating: ", rating
        except:
            #no mapping result
            exc_type, exc_value, exc_traceback = sys.exc_info()
            msg = 'Cannot find match for location query "'+ component.location+'" of wuclass "'+ component.type+ '".' 
            logger.warnMappingStatus(msg)
            set_wukong_status(msg)
            component.message = msg
            candidates, rating = [], []
            mapping_result = False
            continue
            #candidates = locTree.root.getAllAliveNodeIds()
        # construct wuobjects, instances of component
        for candidate in candidates:
            wuclassdef = WuObjectFactory.wuclassdefsbyname[component.type]
            node = locTree.getNodeInfoById(candidate)
            available_wuobjects = [wuobject for wuobject in node.wuobjects.values() if wuobject.wuclassdef.id == wuclassdef.id]
            
            has_wuclass = wuclassdef.id in node.wuclasses.keys()
            wuobj_found = False
            for avail_wuobj in available_wuobjects:
                print avail_wuobj , "avail_wuobj"
                # use existing native wuobject, caution given to obj mapped due to previous candidates
                if not avail_wuobj.virtual and not avail_wuobj.mapped:
                    print avail_wuobj.virtual, avail_wuobj.mapped
                    print "using native at", node.id
                    component.instances.append(avail_wuobj)
                    avail_wuobj.mapped = True
                    wubj_found = True
                    break
                    
            if has_wuclass and (not wuobj_found):    # create a new wuobject from existing wuclasses published from node
                sensorNode = locTree.sensor_dict[node.id]
                sensorNode.initPortList(forceInit = False)
                port_number = sensorNode.reserveNextPort()
                wuobject = WuObjectFactory.createWuObject(wuclassdef, node, port_number,False)
                wuobject.created = True
                wuobject.mapped = True
                component.instances.append(wuobject)
                  
            elif (not wuobj_found) and node.type != 'native' and node.type != 'picokong' and node.type != 'virtualdevice' and node.id != 1 and wuclassdef.virtual==True:
                # create a new virtual wuobject where the node 
                # doesn't have the wuclass for it
                sensorNode = locTree.sensor_dict[node.id]
                sensorNode.initPortList(forceInit = False)
                port_number = sensorNode.reserveNextPort()
                wuobject = WuObjectFactory.createWuObject(wuclassdef, node, port_number, True)
                wuobject.created = True
                wuobject.mapped = True
                component.instances.append(wuobject)
                
        if len(component.instances) < component.group_size:
            msg = 'There is not enough candidates wuobjects from %r for component %s' % (candidates, component.type)
            set_wukong_status(msg)
            logger.warnMappingStatus(msg)
            component.message = msg
            mapping_result = False
            continue
        
        #this is ignoring ordering of policies, eg. location policy, should be fixed or replaced by other algorithm later--- Sen
        component.instances = sorted(component.instances, key=lambda wuObject: wuObject.virtual, reverse=False)
        # limit to min candidate if possible
        # here is a bug if there are not enough elements in instances list   ---Sen
        component.instances = component.instances[:component.group_size]
        for inst in component.instances[component.group_size:]:     #roll back unused virtual wuclasses created in previous step
          if inst.created:
            inst.wunode.port_list.remove(inst.port_number)
            WuObjectFactory.remove(inst.wunode, inst.port_number)
          inst.mapped = False
            
        print ("mapped node id",[inst.wunode.id for inst in component.instances])

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
    print "mapping_result: ",mapping_result

    if flag == None:
        save_map("changesets.pkl",changesets)
        #print changesets.links[0].from_component.location
        #print changesets.links[0].from_property_name, " ", changesets.links[0].from_property.name
        #print changesets.links[0].to_component.location, " " , changesets.links[0].to_property_name, " " , changesets.links[0].to_property 

    return mapping_result

def Compare_changesets (new_changesets, old_changesets):
    diff = ChangeSets(components = [], links = [], heartbeatgroups = [])
    same = ChangeSets(components = [], links = [], heartbeatgroups = [])
    tmp = [] #components will be used
    conflict = False

    for new_l in new_changesets.links:
        flag = 0
        for old_l in old_changesets.links:
            if new_l.from_component.location == old_l.from_component.location and new_l.to_component.location == old_l.to_component.location and new_l.from_component.type == old_l.from_component.type and new_l.to_component.type == old_l.to_component.type:  
                same.links.append(new_l)
                flag = 1
        if flag == 0:
            if (new_l.from_component.location,new_l.from_component.type) not in tmp:
                tmp.append((new_l.from_component.location,new_l.from_component.type))
            if (new_l.to_component.location,new_l.to_component.type) not in tmp:
                tmp.append((new_l.to_component.location,new_l.to_component.type))
            diff.links.append(new_l)

    print "objects will be used:", tmp
    for new_c in new_changesets.components:
        flag = 0

        if new_c.type == "Server":
            diff.components.append(new_c)
            flag = 1
        else:
            for values in tmp:
                if new_c.location == values[0] and new_c.type == values[1]:
                    diff.components.append(new_c)
                    flag = 1
                    break
        if flag == 0:
            for old_c in old_changesets.components:
                if new_c.location == old_c.location and new_c.type == old_c.type:
                    same.components.append(new_c)
                    flag = 1
                    break
            if flag == 0:
                diff.components.append(new_c)
    #check conflict
    for old_l in old_changesets.links:
        if (old_l.from_component.location, old_l.from_component.type) in tmp or (old_l.to_component.location, old_l.to_component.type) in tmp:
            #share policy have bugs
            if share_policy().demo(old_l):
                diff.links.append(old_l)
            else:
                conflict = True
                break

    extra_component = []
    extra_links = []
    for values in tmp:  
        for old_l in old_changesets.links:
            if old_l.from_component.location == values[0] or old_l.to_component.location == values[0]:
                if old_l not in extra_links:
                    extra_links.append(old_l)
        for old_c in old_changesets.components:
            if old_c.location == values[0]:
                if old_c.location not in extra_component:
                    extra_component.append(old_c)
    #for tmp2 in diff.links:
    #    print "diff from:",tmp2.from_component.location,"to:",tmp2.to_component.location

    return (same,diff,extra_component,extra_links,conflict)

def least_changed(logger, changesets, routingTable, locTree):
    try:
        past_changesets = load_map("changesets.pkl")
        past_changesets = unintstall(past_changesets, locTree)
    except:
        mapping_result = firstCandidate(logger, changesets, routingTable, locTree)
        return mapping_result
    same, diff, extra_component, extra_links,conflict = Compare_changesets(changesets, past_changesets) 
    print "same: ", same, "\ndiff: ", diff, "\nextra link:", extra_links, "\nextra component:",extra_component
    if diff.components != []:
        if conflict:
            print "wuobject conflict!"
            mapping_result = firstCandidate(logger, changesets, routingTable, locTree)
        else:
            mapping_result = firstCandidate(logger, diff, routingTable, locTree, 1)
            if extra_component != []:
                for tmp in extra_component:
                    changesets.components.append(tmp)
            if extra_links != []:
                for tmp in extra_links:
                    changesets.links.append(tmp)
            dump_changesets(changesets)
            #same = diff + same
            for tmp in diff.components:
                same.components.append(tmp)
            for tmp in diff.links:
                if tmp not in same.links:
                    same.links.append(tmp)
            for tmp in diff.heartbeatgroups:
                same.heartbeatgroups.append(tmp)
            save_map("changesets.pkl",same)
    else :
        mapping_result = False
        print "no difference"
    return mapping_result

#return True if object can be shared 
class share_policy(object):
    def __init__(self):
        try:
            self.shared_links = load_map("shared_link.pkl")
        except:
            self.shared_links = {}
#        self._unused_wuobj = {}
#        self._used_wuobj = {}
#        i = 1
#        j = 1
#        for nodeid in WuNode.node_dict:
#            node = locTree.getNodeInfoById(nodeid)
#            if node != None:
#                for wuobj in node.wuobjects.values():
#                    if wuobj.mapped == True:
#                        self._used_wuobj.update({i:wuobj})
#                        i +=1
#                    else:
#                        self._unused_wuobj.update({j:wuobj})
#                        j +=1

    def demo(self,link):
        if link.to_component.type == "Server":
            return True
        elif link.from_property.access == "readonly" or link.from_property.access == "readwrite":
            print "readonly and readwirte are sharable."
            self.link_count()
            return True
        else:
            return False

    def pessimistic(self,link): #only readonly components can be shared
        if link.to_component.type == "Server":
            return True
        elif link.from_property.access == "readonly":
            self.link_count()
            return True
        else:
            return False

    def link_count(self):
        tmp = link.from_component.location + link.from_component.type + link.to_component.location + link.to_component.type
        if tmp in self.shared_links:
            self.shared_links[tmp] += 1
        else:
            self.shared_links.update({tmp:1})
        save_map("shared_links.pkl", self.shared_links)

def unintstall(changesets,locTree):
    #To do:change xml.state to unused
    node_id_list = []
    rm_component = []
    for node_id in WuNode.node_dict:
        node_id_list.append(node_id)
    print "node id list:" ,node_id_list
    for component in changesets.components:
        if component.instances[0].wunode.id not in node_id_list:
            rm_component.append(component.location)
            changesets.components.remove(component)
    for link in changesets.links:
        if link.to_component.location in rm_component or link.from_component.loction in rm_component:
            changesets.links.remove(link)
    return changesets

def save_map(name,msg):
    with open(name,"wb") as f:
        pickle.dump(msg,f, pickle.HIGHEST_PROTOCOL)
def load_map(name):
    with open(name,"rb") as f:
        return pickle.load(f)

def dump_changesets(changesets):
    for component in changesets.components:
        print "location: ",component.location, "type:", component.type
    for link in changesets.links:
        print "from location:" , link.from_component.location, "to location:", link.to_component.location
        print "from type:", link.from_component.type, "to type:", link.to_component.type


def delete_application(logger, changesets, routingTable, locTree):
    import os
    try:
        past_changesets = load_map("changesets.pkl")
        past_changesets = unintstall(past_changesets, locTree)
    except:
        print "file does not exist"

    try:
        shared_links = load_map("shared_link.pkl")
    except:
        shared_links = {}

    #delete dir & files    
    app_dir = changesets.component.application_hashed_name
    print "delete: " ,app_dir
    os.shutil.rmtree( os.getcwd() +"../apps/" + app_dir)

    #compare changesets
    delete = []
    remap = ChangeSets(components = [], links = [], heartbeatgroups = [])
    tmp = [] #components will be used
    conflict = False

    for del_l in changesets.links:
        flag = 0
        tmp_str = del_l.from_component.location + del_l.from_component.type + del_l.to_component.location + del_l.to_component.type
        shared_links[tmp_str] -= 1
        if shared_links[tmp_str] == 0:
            shared_links.pop(tmp_str)
            delete.append(tmp_str)
        if del_l.from_component.location not in tmp:
            tmp.append(del_l.from_component.location)
        if del_l.to_component.location not in tmp:
            tmp.append(del_l.to_component.location)

    print "wudevices need redeployed:", tmp

    for component in past_changesets.components:
        if components.location in tmp:
            remap.components.append(component)
    for link in past_changesets.links:
        if link.from_component.location in tmp or link.to_component.location in tmp:
            tmp_str = link.from_component.location + link.from_component.type + link.to_component.location + link.to_component.type
            if tmp_str not in delete:
                remap.links.append(link)
    changesets = remap
    return True

