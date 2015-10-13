#the children of leaf nodes are sensor nodes, sensor nodes

import logging
import json
import ast
#import numpy
import math

MAX_LIFE = 1
class LandmarkNode:
    #location is a string, size is a tuple of three (corresponding to x, y,z direction)
    #assuming shapes to be cuboid, but it could also be cylinder, pyramid, ball etc
    #size, direct are tuples
    currentID = 0
    def __init__(self, name, locationStr, size, direct = None):
        self.id = LandmarkNode.currentID
        LandmarkNode.currentID = LandmarkNode.currentID+1
        self.name = name
        self.location = locationStr
        self.locationTreeNode = None
        self.locationLst,x_coord,y_coord, z_coord = LocationTree.parseLocation(locationStr)
        self.coord = (x_coord,y_coord,z_coord)
        self.size = size
        self.direction = direct
        self.json_data = []
    def getId(self):
        return self.name

class SensorNode:
    def __init__(self, nodeInfo):
        self.location = nodeInfo.location
        self.locationLst, x_coord, y_coord, z_coord = LocationTree.parseLocation(nodeInfo.location)
        self.locationTreeNode = None
        self.nodeInfo = nodeInfo
        self.coord = (x_coord,y_coord,z_coord)      #local coord
        self.life = MAX_LIFE
        self.port_list = []
        self.temp_port_list = []
        self.last_reserved_port = 0

    def getGlobalCoord(self):
        if self.locationTreeNode!= None:
            return self.locationTreeNode.transformToGlobal(self.coord)
        else:
            return self.coord

    def initPortList(self, forceInit = True):
        if len(self.port_list)!=0 and forceInit == False:
            return
        self.port_list = []
        for wuObj in self.nodeInfo.wuobjects.values():
            self.port_list.append(wuObj.port_number)

        if (len(self.port_list) == 0):
            self.port_list.append(1);
        self.port_list.sort()
    def reserveNextPort(self):      #reserve a port from 0 ~ 127
        portSet = False
        for j in range(len(self.port_list)-1 ,-1 , -1):
            if (self.port_list[j]+1)%128 !=self.port_list[(j+1)%len(self.port_list)]:
                self.port_list.append((self.port_list[j]+1)%128)
                self.temp_port_list.append((self.port_list[j]+1)%128)
                self.port_list.sort()
                portSet =True
                self.last_reserved_port = (self.port_list[j]+1)%128
                return (self.port_list[j]+1)%128
        return None
    def isAlive(self):
        return self.life == MAX_LIFE

class LocationTreeNode:
    def __init__(self, name, parent):
        self.id = 1
        if parent != None:
            if self in parent.children:
                self.id = 100*parent.id + parent.children.index(self)
            else:
                self.id = 100*parent.id + len(parent.children)
        self.name = name
        self.parent = parent
        self.children = []
        self.childrenCnt = 0
        self.sensorLst = []
        self.sensorCnt = 0
        self.landmarkLst = []
        self.size = (0,0,0)
        self.originalPnt = (0,0,0)     #in coord system of parent
        self.centerPnt = (0,0,0)      #in coord system of parent
        self.direction = None
        #represent [[cosx1x2, cosx2y1, cosx2z1],[cosy1x2, cosy1y2,cosy1z2],[cosz1x2,cosz1y2,cosz1z2]]
        #globalCoord = transMatrix * localCoord
        self.transMatrix = [[1,0,0],[0,1,0],[0,0,1]]   #default no transformation between coordinate system

        #key is the set of the destination and source (exchangeable)
        #distance to self is always 0
        self.distanceModifier = {} #stores a list of distance between children, default 0, used for distance between sensors in different children.
        self.idSet = set([]) #all sensor ids contained in this Node and its children nodes

        '''    obsolete requires numpy
            def transformToLocal(self, vect):
                A = numpy.matrix(self.transMatrix)
                inverseA = A.I
                return tuple((vect*inverseA).tolist()[0])
        '''
    def getLandmarkList(self):
        lmlst = []
        for landmk in self.landmarkLst:
            lmlst.append([landmk.name,str(landmk.coord)])
        return lmlst

    def distanceModifierToString(self):
        tmpDict = {}
        ret_str = '{'
        for child in self.children:
            tmpDict[child.id] = child.name
        for k in self.distanceModifier.keys():
            ret_str += '"(' + str(tmpDict[k[0]]) +', '+ str(tmpDict[k[1]]) + ')":' + str(self.distanceModifier[k]) +', '
        if len(ret_str)>1 and ret_str[-2] ==',':
            ret_str = ret_str[:-2]
        ret_str +='}'
        return ret_str

    def distanceModifierIdToString(self):
        tmpDict = {}
        ret_str = '{'
        for k in self.distanceModifier.keys():
            ret_str += '"[' + str(k[0]) +', '+ str(k[1]) + ']":' + str(self.distanceModifier[k]) +', '
        if len(ret_str)>1 and ret_str[-2] ==',':
            ret_str = ret_str[:-2]
        ret_str +='}'
        return ret_str

    #set local Global Size Direction from strings
    def setLGSDFromString(local_str, global_str, size_str, direction_str):
        self.originalPnt = ast.literal_eval(local_str)
        self.size = ast.literal_eval(size_str)
        if direction_str != "None":
            self.direction = ast.literal_eval(direction_str)
        else:
            self.direction = None

    #transMatrix is a 3*3 nested list
    def setLocalTransMatrix (self, transMatrix):
        self.localTransMatrix = transMatrix

    #transform a local vector (x,y,z) to global one
    def transformToGlobal(self, vect):
        print "vect:", vect
        newVec = [0,0,0]
        for i in range(3):
            for j in range(3):
                newVec[i] = newVec[i]+ self.transMatrix[i][j]*vect[j]
        return tuple(newVec)

    #originalPnt is a tuple of 3, e.g.(0,1,2)
    def setOriginalPnt (self, originalPoint):
        self.originalPnt = originalPoint

    def getGlobalOrigPnt(self):
        pa_global = (0,0,0)
        curNd = self
        while curNd.parent !=None:
            curNd = curNd.parent
            transformedCoord = curNd.transformToGlobal(self.originalPnt)
            pa_global = (pa_global[0] + transformedCoord[0],
                         pa_global[1] + transformedCoord[1],
                         pa_global[2] + transformedCoord[2])
        transformedCoord = curNd.transformToGlobal(self.originalPnt)
        pa_global = (curNd.originalPnt[0] + transformedCoord[0],
                     curNd.originalPnt[1] + transformedCoord[1],
                     curNd.originalPnt[2] + transformedCoord[2])
        return pa_global

    def getOriginalPnt (self):
        return self.originalPnt
    def getCenterPnt(self):
        return self.centerPnt
    def setCenterPnt (self, centerPnt):
        self.centerPnt = centerPnt

    #obj could be sensor or landmark
    def calcDistance(self, sensor, obj):
#        sensorGlobalCoord = (0,0,0)
#        objGlobalCoord = (0,0,0)
#        for i in range(3):
#            for j in range(3):
#                sensorGlobalCoord[i] += self.transMatrix[i][j]*sensor.coord[j]
#                objGlobalCoord[i] += self.transMatrix[i][j]*obj.coord[j]
 #       modifier = (sensorGlobalCoord[0]-objGlobalCoord[0])**2+(sensorGlobalCoord[1]-objGlobalCoord[1])**2+(sensorGlobalCoord[2]-objGlobalCoord[2])**2
        #if in same room, Euclid distance
        pos2 = sensor.locationTreeNode
        snrId = sensor.nodeInfo.id
        pos1 = obj.locationTreeNode
        distance = 0
        if pos1.id == pos2.id:
            for i in range(3):
                distance = distance + (sensor.coord[i] - obj.coord[i])**2
            distance = math.sqrt(distance)
            return distance

        #if in different rooms
        distance1, distance2 = 0, 0
        for i in range(3):
            distance1 = distance1 + sensor.coord[i]**2
            distance2 = distance2 + obj.coord[i]**2
        distance1 = math.sqrt(distance1)
        distance2 = math.sqrt(distance2)
        distance = distance1 + distance2

        curPos = pos1
        while snrId not in curPos.idSet:
            try:
                distance = distance +self.distanceModifier[(pos1.id,curPos.id)]
            except KeyError:    #default barrier between different nodes of different layer is 1000
                distance = distance+0
            pos1 = curPos
            curPos = curPos.parent
        try:
            distance = distance +curPos.distanceModifier[(pos1.id,pos2.id)]
        except KeyError:     #default barrier between different nodes of the same layer is 0
            distance = distance+1000
        return distance

    def addDistanceModifier(self, id1, id2, distance):
        found = 0
        ids = [id1, id2]
        lst = self.children
     #   if self.parent !=None:
      #      lst.append(self.parent)
        for child in lst:
            if child.id == id1:
                found = found+1
                if found == 3:
                    break
            elif child.id == id2:
                found = found +2
                if found == 3:
                    break
        if found == 0:
            logging.error("error! None of the ids"+str(ids)+" found when adding distance modifier to " + str(self.id))
            return False
        elif found!=3:
            logging.error("error! Id"+str(ids[2-found])+" not found when adding distance modifier to"+ str(self.id))
            return False

        self.distanceModifier[(id1,id2)] = distance
        self.distanceModifier[(id2,id1)] = distance
        return True

    def delDistanceModifier(self, id1, id2, distance):
        found = 0
        ids = [id1, id2]
        lst = self.children
        if self.parent !=None:
            lst.append(self.parent)
        for child in lst:
            if child.id == id1:
                found = found+1
                if found == 3:
                    break
            elif child.id == id2:
                found = found +2
                if found == 3:
                    break
        if found == 0:
            logging.error("error! None of the ids"+str(ids)+" found when adding distance modifier to " + self.id)
            return False
        elif found!=3:
            logging.error("error! None of the ids"+str(ids[2-found])+" found when adding distance modifier to"+ self.id)
            return False
        try:
            del self.distanceModifier[(id1,id2)]
            del self.distanceModifier[(id2,id1)]
        except KeyError:
            logging.error("error! distance modifier does not exist for " +str(ids)+" in "+ self.id)
            return False
        return True

    def findChildByName(self, name):
        for child in self.children:
            if child.name == name:
                return child
        return None

    def addChild(self, name):
        tmp = LocationTreeNode (name, self)
        self.children.append(tmp)
        self.childrenCnt = self.childrenCnt + 1

    def delChild(self, locTreeNode):
        self.children.remove(locTreeNode)
        self.childrenCnt = self.childrenCnt - 1

    def addSensor(self, sensorNode):
        self.sensorLst.append(sensorNode)
        self.sensorCnt = self.sensorCnt + 1
        sensorNode.locationTreeNode = self
        self.idSet.add(sensorNode.nodeInfo.id)
        # update sensorCnt for all ancestor nodes
        pa = self.parent
        while pa != None:
            pa.sensorCnt = pa.sensorCnt + 1
            pa.idSet.add(sensorNode.nodeInfo.id)
            pa = pa.parent

    def getSensorById (self, Id): #return None on failure
        curNode = self
        while curNode != None and (Id in curNode.idSet):
            found = False
            for child in curNode.children:
                if Id in child.idSet:
                    curNode = child
                    found = True
                    break
            if found == False: #sensor in curNode but not in its children
                for senr in curNode.sensorLst:
                    if senr.nodeInfo.id == Id:
                        return senr
                curNode = None
        return None

    def addLandmark(self, landmarkNode):
        if landmarkNode not in self.landmarkLst:
            for existingLandmarkNd in self.landmarkLst:
                if existingLandmarkNd.name == landmarkNode.name:
                    self.landmarkLst.remove(existingLandmarkNd)
                    del existingLandmarkNd
                    break
            self.landmarkLst.append(landmarkNode)
            landmarkNode.locationTreeNode = self
            return True
        return False

    def delLandmark (self, landmarkId):
        for landmarkNd in self.landmarkLst:
            if landmarkId == landmarkNd.name:
                self.landmarkLst.remove(landmarkNd)
                del landmarkNd
                return True
        return False

    def findLandmarksByName(self, landmarkName):
        retLst = []
        for landmarkNd in self.landmarkLst:
            if landmarkName == landmarkNd.name:
                retLst.append(landmarkNd)
        for child in self.children:
            retLst = retLst + child.findLandmarksByName(landmarkName)
        return retLst

    def findLandmarkById(self, landmarkId):
        for landmarkNd in self.landmarkLst:
            if landmarkId == landmarkNd.id:
                return landmarkNd
        for child in self.children:
            result = child.findLandmarkById(landmarkId)
            if result != None:
                return result
        return None

    def delSensor(self, sensorNode):
        self.sensorLst.remove(sensorNode)
        self.sensorCnt = self.sensorCnt - 1
        sensorNode.locationTreeNode = None
        self.idSet.remove(sensorNode.nodeInfo.id)
        # update sensorCnt for all ancestor nodes
        pa = self.parent
        while pa != None:
            pa.sensorCnt = pa.sensorCnt - 1
            pa.idSet.remove(sensorNode.nodeInfo.id)
            pa = pa.parent

    def getAllNodes(self):
        ret_val = self.idSet
        return ret_val

    def getAllAliveNodeIds(locTreeNode = None):
        if locTreeNode == None:
            locTreeNode = self
        tmpLst = []
        for sensor in locTreeNode.sensorLst:
            if sensor.isAlive():
                tmpLst.append(sensor.nodeInfo.id)
        tmpLst = set(tmpLst)
        for child in locTreeNode.children:
            tmpLst = tmpLst | set(child.getAllAliveNodeIds())
        return list(tmpLst)

    def getAllNodeInfos(self):
        ret_val = []
        for sensor in self.sensorLst:
            ret_val.append(sensor.nodeInfo)
        for child in self.children:
            ret_val.append(child.getAllNodeInfos())
        return ret_val

    def getLocationStr(self):
        loc_str = '/'+self.name
        cur_node = self
        while(cur_node.parent !=None):
            loc_str = '/'+ cur_node.parent.name+ loc_str
            cur_node = cur_node.parent
        return loc_str

    def toString(self, indent = 0):
        print_str = ""
        for i in range(indent):
            print_str = print_str + "\t"
        print_str = print_str + self.name + "#"
        for i in range(len(self.sensorLst)):
            print_str = print_str + str(self.sensorLst[i].nodeInfo.id) +str(self.sensorLst[i].coord)+", "

        for landmarkNode in self.landmarkLst:
            print_str = print_str + 'landmark: '
            print_str = print_str + str(landmarkNode.id)+str(landmarkNode.name)+str(landmarkNode.coord)+', '
        return print_str

    def _genJsonArray(self, indent = 0):
        print_str = ""
        print_str = print_str + self.name
        json_data = []
        json_data.append([0,indent, [self.id, print_str]])
        for i in range(len(self.sensorLst)):
            json_data.append([1, indent+1,  [self.sensorLst[i].nodeInfo.id, str(self.sensorLst[i].coord)]])
        for landmarkNode in self.landmarkLst:
            json_data.append([2, indent+1, [str(landmarkNode.name), landmarkNode.id, str(landmarkNode.coord), str(landmarkNode.size)]])
        return json_data

class LocationTree:

    def __init__(self, name):
        tmp = LocationTreeNode(name, None)
        self.sensor_dict = {}
        self.root = tmp
        self.totalSensorCount = 0

    def decreaseSensorLife(self):
        for k in self.sensor_dict.keys():
            sensor = self.sensor_dict[k]
            sensor.life = sensor.life - 1
            if sensor.life == 0:
                self.delSensor(sensor.nodeInfo.id)

    def updateSensors(self, newInfoList):
        #decrease all sensor lifes
        self.decreaseSensorLife()
        for info in newInfoList:
            print info.id
            sensor = SensorNode(info)
            self.addSensor(sensor)



    def delSensor(self, sensorId):
        if sensorId not in self.sensor_dict:
            logging.info("Node",sensorId," not in location tree, deletion ignored")
        sensorNode = self.sensor_dict[sensorId]
        locTreeNode = sensorNode.locationTreeNode

        locTreeNode.delSensor(sensorNode)
        del self.sensor_dict[sensorId]
        self.totalSensorCount = self.totalSensorCount - 1
        #delete unnecessary branches in the tree (del branches with no sensor node)
   #     while locTreeNode.sensorCnt == 0 and len(locTreeNode.landmarkLst)==0:
    #        pa = locTreeNode.parent
     #       if pa != None:
      #          pa.delChild(locTreeNode)
    #        else: #root of the tree
     #           break
      #      del locTreeNode
       #     locTreeNode = pa

    def getAllNodeInfos(self):
        return self.root.getAllNodeInfos()

    def getAllAliveNodeIds():
        tmpLst = []
        for key in self.sensor_dict.keys():
            if self.sensor_dict(key).isAlive() == True:
                tmpLst.append(key)
        return tmpLst

        #landmarks must be added into already created location tree node
    def addLandmark(self, landmarkNd, startPos = None):
        if startPos == None:
            startPos = self.root
        if landmarkNd.locationLst == None or len(landmarkNd.locationLst) == 0:
            logging.error("error! location for node "+
                str(landmarkNd.nodeInfo.id)+ " is not set")
            return False
        if startPos.name != landmarkNd.locationLst[0]:
            logging.error("error! location: "+ str(landmarkNd.locationLst[0])+ " does not match " + startPos.name)
            return False
        curPos = startPos
        for i in range(1, len(landmarkNd.locationLst)):

            if curPos.childrenCnt==0:
                curPos.addChild(landmarkNd.locationLst[i])
                curPos = curPos.children[-1]
            else:
                child_index = -1
                for j in range(curPos.childrenCnt):
                    if curPos.children[j].name == landmarkNd.locationLst[i]:
                        child_index = j
                if (child_index >= 0):
                    curPos = curPos.children[child_index]
                else:
                    curPos.addChild(landmarkNd.locationLst[i])
                    curPos = curPos.children[-1]

        curPos.addLandmark(landmarkNd)
        return True

    def delLandmark(self, landmarkId, locationStr):

        logging.info("Node"+str(landmarkId)+" not in location tree, deletion ignored")
        locTreeNode = self.findLocation(self.root, locationStr)
        #print locTreeNode, "in dellandmark"
        rt_val = False
        if locTreeNode:
            print ("found loctreeNode for dellandmark", landmarkId)
            rt_val = locTreeNode.delLandmark(landmarkId)
            print ("dellandmark returns", rt_val)
        return rt_val
        #delete unnecessary branches in the tree (del branches with no sensor node)
  #      while locTreeNode.sensorCnt == 0 and len(locTreeNode.landmarkLst)==0:
  #          pa = locTreeNode.parent
  #          if pa != None:
  #              pa.delChild(locTreeNode)
  #          else: #root of the tree
  #              break
  #          del locTreeNode
  #          locTreeNode = pa

    #save tree structure and landmarks
    def saveTree(self, filename="../LocalData/landmarks.txt"):
        landmark_str =""
        fin = open(filename,"w")
        landmark_str += self.__saveTreeNode(self.root, fin)
        fin.write( "Landmarks:\n" + landmark_str)
        fin.close()

    def loadTree(self, filename="../LocalData/landmarks.txt"):
        fout = open(filename, "r")
        content = fout.read()
        location_strs,landmark_strs = content.split("Landmarks:")
        locations = location_strs.split('\n')
        landmarks = landmark_strs.split('\n')
        for i in range(len(locations)):
            if len(locations[i]) > 0:
                [location_str, modifiers] = locations[i].split("::")
                modifiers = ast.literal_eval(modifiers)
                newLocTreeNode = self.addLocation(location_str)
                newLocTreeNode.distanceModifier = modifiers
        for i in range(len(landmarks)):
            if len(landmarks[i]) > 0:
                location_str, name = landmarks[i].split("::")
                landmarkNd = LandmarkNode(name, location_str, None)
                self.addLandmark(landmarkNd)
        self.printTree()

    #used by saveTree for recursively saving tree structure and landmarks
    def __saveTreeNode(self, treeNode, fin):
        currentLocation = treeNode.getLocationStr()
        landmark_str = ''
        fin.write(currentLocation+"::"+str(treeNode.distanceModifier)+"\n")
        for i in range(len(treeNode.landmarkLst)):
            landmark_str += treeNode.landmarkLst[i].location + "::" + treeNode.landmarkLst[i].name+"\n"
        for i in range(treeNode.childrenCnt):
            landmark_str += self.__saveTreeNode(treeNode.children[i], fin)
        return landmark_str


    #addLocation returns the locationTreeNode added
    def addLocation(self, locationStr, startPos = None):
        if startPos == None:
            startPos = self.root
        locationLst,x,y,z = self.parseLocation(locationStr)
        if locationLst == None or len(locationLst) == 0:
            logging.error("error! location is not given correctly")
            return False

        if startPos.name != locationLst[0]:
            if startPos == self.root:
                locationLst = [self.root.name] + locationLst
            else:
                logging.error("error! location: "+ str(locationLst[0])+ " of node does not match expected" + startPos.name)
                print ("error! location: "+ str(locationLst[0])+ " of node does not match expected" + startPos.name)
                return False
        curPos = startPos
        for i in range(1, len(locationLst)):

            if curPos.childrenCnt==0:
                curPos.addChild(locationLst[i])
                curPos = curPos.children[-1]
            else:
                child_index = -1
                for j in range(curPos.childrenCnt):
                    if curPos.children[j].name == locationLst[i]:
                        child_index = j
                if (child_index >= 0):
                    curPos = curPos.children[child_index]
                else:
                    curPos.addChild(locationLst[i])
                    curPos = curPos.children[-1]
        return curPos

    #insert sensorNd into the tree with its location specified in locationLst, starting from startPos node(set to root if locationLst start from beginning)
    def addSensor(self, sensorNd, startPos = None ):
        if startPos == None:
            startPos = self.root

        if sensorNd.nodeInfo.id in self.sensor_dict:
            if sensorNd.nodeInfo.location == self.sensor_dict[sensorNd.nodeInfo.id].location:
                self.sensor_dict[sensorNd.nodeInfo.id].life = MAX_LIFE
                self.sensor_dict[sensorNd.nodeInfo.id].nodeInfo = sensorNd.nodeInfo
                self.sensor_dict[sensorNd.nodeInfo.id].port_list = sensorNd.port_list
                self.sensor_dict[sensorNd.nodeInfo.id].locationTreeNode = sensorNd.locationTreeNode
                return True
            else: #sensor node location needs to be updated, delete the original inserted SensorNd first
                self.delSensor(sensorNd.nodeInfo.id)
        if sensorNd.locationLst == None or len(sensorNd.locationLst) == 0:
            logging.error("error! location for node "+ str(sensorNd.nodeInfo.id)+ " is not set")
            return False

        if startPos.name != sensorNd.locationLst[0]:
            if startPos == self.root:
                sensorNd.locationLst = [self.root.name] + sensorNd.locationLst
            else:
                logging.error("error! location: "+ str(sensorNd.locationLst[0])+ " of node does not match expected" + startPos.name)
                print ("error! location: "+ str(sensorNd.locationLst[0])+ " of node does not match expected" + startPos.name)
                return False
        curPos = startPos
        for i in range(1, len(sensorNd.locationLst)):

            if curPos.childrenCnt==0:
                curPos.addChild(sensorNd.locationLst[i])
                curPos = curPos.children[-1]
            else:
                child_index = -1
                for j in range(curPos.childrenCnt):
                    if curPos.children[j].name == sensorNd.locationLst[i]:
                        child_index = j
                if (child_index >= 0):
                    curPos = curPos.children[child_index]
                else:
                    curPos.addChild(sensorNd.locationLst[i])
                    curPos = curPos.children[-1]

        curPos.addSensor(sensorNd)
        self.sensor_dict[sensorNd.nodeInfo.id] = sensorNd
        self.totalSensorCount = self.totalSensorCount +1
        return True
    def deleteLocation(self, locationStr, startPos = None):
        if startPos == None:
            startPos = self.root
        locTreeNode = self.findLocation(startPos, locationStr)
        pa = locTreeNode.parent
        if pa != None:  #we don't delete root
          pa.delChild(locTreeNode)
          del locTreeNode

    def getNodeInfoById (self, Id): #return None on failure
        curNode = self.root
        while curNode != None and (Id in curNode.idSet):
            found = False
            for child in curNode.children:
                if Id in child.idSet:
                    curNode = child
                    found = True
                    break
            if found == False: #sensor in curNode but not in its children
                for senr in curNode.sensorLst:
                    if senr.nodeInfo.id == Id:
                        return senr.nodeInfo
                curNode = None
        return None

    def getSensorById (self, Id): #return None on failure
        curNode = self.root
        while curNode != None and (Id in curNode.idSet):
            found = False
            for child in curNode.children:
                if Id in child.idSet:
                    curNode = child
                    found = True
                    break
            if found == False: #sensor in curNode but not in its children
                for senr in curNode.sensorLst:
                    if senr.nodeInfo.id == Id:
                        return senr
                curNode = None
        return None
    def findLocationById(self, node_id):
        idLst = []
       #each number in the list will indicate which child of the tree to go into to find the node
        while node_id>=100:
          idLst.append(node_id%100)
          node_id = node_id//100

        idLst.append(node_id)
        idLst.reverse()
        idLst = idLst[1:]       #get rid of heading 1 (which means root)
        curNode = self.root
        for i in idLst:
          if len(curNode.children)> i:
            curNode= curNode.children[i]
          else:
            return None
        return curNode

    def findLocation(self, startPos, locationStr):
        locationLst = None
        if type(locationStr) == list:
            locationLst = locationStr       #in locationParser, we may pass a list directly
        else:
            locationLst,x,y,z = self.parseLocation(locationStr)
        if startPos.name != locationLst[0]:
            logging.error("error! location: "+ str(locationLst[0])+ " is not a valid value")
            return None
        curPos = startPos

        for i in range(1, len(locationLst)):
            if curPos.childrenCnt==0:
                return None
            else:
                child_index = -1
                for j in range(curPos.childrenCnt):
                    if curPos.children[j].name == locationLst[i]:
                            child_index = j
                if (child_index >= 0):
                    curPos = curPos.children[child_index]
                else:
                    return None
        return curPos

    @staticmethod
    def parseLocation (locationStr):
      #be able to handle something like /CS_Building/4F/Room336@(1,2,3)
        tmpLst = locationStr.split(u'@')
        x_coord,y_coord,z_coord = '0','0','0'
        if len(tmpLst)>1:
            ff = tmpLst[1].rstrip(') ').lstrip('( ').split(',')
            if len(ff) == 3:
                [x_coord,y_coord,z_coord] = tmpLst[1].rstrip(') ').lstrip('( ').split(',')
        else:
            tmpLst = tmpLst[0].split(u'#')
        locationLst = tmpLst[0].split(u'/')
        for loc in locationLst:
            if len(loc) == 0:
                locationLst.remove(loc)
        try:
            x_coord = int(x_coord)
            y_coord = int(y_coord)
            z_coord = int(z_coord)
        except ValueError:
            x_coord = 0
            y_coord = 0
            z_coord = 0
        return locationLst, x_coord, y_coord, z_coord

    def printTree(self, treeNd=None, indent = 0):

        str =""
        if treeNd ==None:
            treeNd = self.root
        str += treeNd.toString(indent)

        print (str)

        for i in range(treeNd.childrenCnt):
            self.printTree(treeNd.children[i], indent+1)

    def getJson(self, treeNd=None, indent = 0):
        if treeNd ==None:
            self.json_data = []
            treeNd = self.root
        self.json_data.extend(treeNd._genJsonArray(indent))
        for i in range(treeNd.childrenCnt):
            self.getJson(treeNd.children[i], indent+1)
        return self.json_data

    def buildTree(self, node_infos):
        self.updateSensors(node_infos)

 # for info in node_infos:
  # senNd = SensorNode(info)
   # self.addSensor(senNd)

if __name__ == "__main__":

    loc4 = u"asfafaeff@gaerg"
    print(LocationTree.parseLocation(loc4))
    locTree = LocationTree(u"Boli_Building")
    loc0 = u"Boli_Building/3F/South_Corridor@(0,1,2)"
    loc1 = u"Boli_Building/2F/South_Corridor/Room318@(0,5,3)"
    loc2 = u"Boli_Building/3F/East_Corridor/Room318@(3,3,2)"
    loc3 = u"Boli_Building/3F/East_Corridor/Room318@(2,1,2)"

    senNd0 = SensorNode(NodeInfo(0,[], [], loc0))

    senNd1 = SensorNode(NodeInfo(1, [], [], loc1))
    senNd2 = SensorNode(NodeInfo(2, [], [], loc2))
    senNd3 = SensorNode(NodeInfo(3, [], [], loc3))
    landmark1 = LandmarkNode(0, u'sofa',u"Boli_Building/3F/East_Corridor/Room318@(2,1,2)", (2,2,1))
    landmark2 = LandmarkNode(1, u'sofa',u"Boli_Building/3F/East_Corridor/Room319@(2,1,2)", (2,2,1))

    infoList = [NodeInfo(0,[], [], loc0),NodeInfo(1,[], [], loc1),NodeInfo(2,[], [], loc3),NodeInfo(4,[], [], loc2)]
    print(locTree.addSensor(senNd0, locTree.root))
    locTree.addSensor(senNd1)
    locTree.addSensor(senNd2)
    locTree.addSensor(senNd3)
    locTree.addLandmark(landmark1)
    locTree.addLandmark(landmark2)
    locTree.printTree(locTree.root, 0)
    locTree.updateSensors(infoList)
    locTree.printTree()
    locTree.delLandmark (0, u"Boli_Building/3F/East_Corridor/Room318@(2,1,2)")
    locTree.delLandmark(1,u"Boli_Building/3F/East_Corridor/Room319@(2,1,2)")
    locTree.printTree()
    print locTree.root
    locTree.getAllNodeInfos()
