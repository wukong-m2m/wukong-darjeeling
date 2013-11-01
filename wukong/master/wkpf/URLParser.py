#Sen Zhou 12.8.14 
#define how to parse URLs like EE_Building/3F/ROOM318#near(0,1,2)&inside(3F),  
#(, ), &, |, are preserved key words
#priority | < & < ~
#any kind of spaces, tabs or "\n" are not allowed in URL

from models import Node
from locationTree import *


class LocationParseTreeNode:
    def __init__(self, contentStr):
        self.function = None
        self.childCnt = 0
        self.operation = None
        self.children = []
        self.contentStr = contentStr
        
class LocationURL:
    locationTreeNode = None
    def __init__(self, urlStr, locationTree):
        self.urlStr = urlStr
        self.parseTreeRoot = None
        self.nodeIdSet = set([]) 
        self.locationTree = locationTree
    
    def near(locationTreeNode, dist, x, y=None,z=None):
        ret_val = set([])
        dist = float(dist)
        try:            #case one, coordinates are given
            x = float(x)
            y = float(y)
            z = float(z)
        except ValueError:  #case 2, the name of a landmark is given, x would be the landmarkName
            landMarks = locationTreeNode.findLandmarksByName(x)
            # Assume we use the first landmark of the same name
            if landMarks ==None or len(landMarks)==0:   #no such landmark of the name
                return ret_val
            x = landMarks[0].coord[0]
            y = landMarks[0].coord[1]
            z = landMarks[0].coord[2]
        
        
        for sensorNd in locationTreeNode.sensorLst:
            if (sensorNd.coord[0]-x)**2+(sensorNd.coord[1]-y)**2+(sensorNd.coord[2]-z)**2 <= dist**2:
                #print sensorNd.coord[0],sensorNd.coord[1],sensorNd.coord[2]
                ret_val.add(sensorNd.nodeInfo.nodeId)
        return ret_val

#return the nearest 'count' nodes from idset(or location treenode is idset=None) to x,y,z(or x when x is the landmark name)
    def nearest(locationTreeNode, x, y=None,z=None, count=-1, idLst=None):
        node_lst = []       #list of nodes to be returned
        dist_lst = []       #list of nodes' distances in the node_list
        try:            #case one, coordinates are given
            x = float(x)
            y = float(y)
            z = float(z)
        except ValueError:  #case 2, the name of a landmark is given, x would be the landmarkName
            landMarks = locationTreeNode.findLandmarksByName(x)
            # Assume we use the first landmark of the same name
            if landMarks ==None or len(landMarks)==0:   #no such landmark of the name
                return ret_val
            x = landMarks[0].coord[0]
            y = landMarks[0].coord[1]
            z = landMarks[0].coord[2]

        if count ==-1:
            count = 65535   #setting count==-1 means we want to find and sort all
            
        largest_dist = 0
        if idLst == None:
            idLst = list(locationTreeNode.idSet)
        for sensorId in idLst:
            sensorNd = locationTreeNode.getSensorById(sensorId)
            if sensorNd == None:
                continue
            dist = (sensorNd.coord[0]-x)**2+(sensorNd.coord[1]-y)**2+(sensorNd.coord[2]-z)**2
            if dist >= largest_dist:
                if len(node_lst)<count:
                    #print sensorNd.coord[0],sensorNd.coord[1],sensorNd.coord[2]
                    node_lst.append(sensorNd.nodeInfo.nodeId)
                    dist_lst.append(dist)
                    largest_dist = dist
            else:
                for i in range(len(node_lst)):
                    if dist_lst[i] > dist:
                        node_lst.insert(i, sensorNd.nodeInfo.nodeId)
                        dist_lst.insert(i, dist)
                        break
        return node_lst
        
    def isID(locationTreeNode, id):
        id = int(id)
        return set([id])
        
    def getIntersect(locationTreeNode,  set1, set2):
        return set1 & set2

    def getUnion(locationTreeNode,  set1, set2):
        return set1 | set2

    def negate(locationTreeNode, set1):
        return locationTreeNode.idSet - set1
        
    def getAll(locationTreeNode):
        return locationTreeNode.getAllAliveNodeIds()
    
    connector_lst=[ u"|", u"&", u"~"]
    connector_dict = {u"|":[getUnion,2], u"&":[getIntersect,2], u"~":[negate,1]}  #for DNF(disjunctive norm form expression), function call and num of variables
    funct_dict = {u"near":near, u"idID":isID, u"getAll":getAll, u"nearest":nearest}

    
        
    def _isFuncParameter(self, numStr):
        try:
            numPara = float(numStr)
        except ValueError:
            return False
        return True
    
    def _isFuncName(self, nameStr):
        if self.funct_dict.has_key(nameStr):
            return True
        return False
    
    def _genFunction(self,functionStr):
        tmpLst = functionStr.split('(')
        func_name = tmpLst[0].strip()
        if self._isFuncName(func_name) == False:
            print "Error while parsing url:" + self.urlStr +", no function named "+ func_name+" found!"
            return None
        foo = URLFunction(func_name, self.funct_dict[func_name], self.locationTreeNode)
        argStr = tmpLst[1].strip(" ),")
        if len(argStr)>0:
            foo.args += argStr.split(',')
        #print foo.args
        return foo
    
    #parameters: str --- string to be analysis, connector_id ---- seq. of connector in connector_lst, treeNode --- parseTreeNode to start with
    def _buildParseTree(self, treeNode, connector_id):
        if connector_id >= len(self.connector_lst):     #has gone through all connectors, begin parse each part to single function
            if len(treeNode.contentStr)>0:
                funct = self._genFunction(treeNode.contentStr)
                treeNode.function = funct
                return 0
            else:
                return -1

        location = treeNode.contentStr.find(self.connector_lst[connector_id])
        if location == -1:      #cannot find desired connector, proceed to find the next connector
            self._buildParseTree(treeNode, connector_id+1)
        else:
            treeNode.operation = self.connector_lst[connector_id]
            if location != 0:           #desired connector is in the middle of the string, go into the first and last half respectively
                treeNode.children.append(LocationParseTreeNode(treeNode.contentStr[:location]))
                #print treeNode.contentStr[:location], connector_id
                self._buildParseTree(treeNode.children[-1], connector_id+1)
            treeNode.children.append(LocationParseTreeNode(treeNode.contentStr[location+len(self.connector_lst[connector_id]):]))
            #print treeNode.contentStr[location+len(self.connector_lst[connector_id]):]
            self._buildParseTree(treeNode.children[-1], connector_id)
        
            
    def solveParseTree(self, treeNode = None):
        if treeNode == None:
            treeNode = self.parseTreeRoot
        if treeNode.function!=None:
            #print treeNode.function.toString()
            return treeNode.function.runFunction()
        if self.connector_dict[treeNode.operation][1] == 1: #unary op
            return self.connector_dict[treeNode.operation][0](self.locationTreeNode, self.solveParseTree(treeNode.children[0]))
        elif self.connector_dict[treeNode.operation][1] == 2:   #binary op
            return self.connector_dict[treeNode.operation][0](self.locationTreeNode,  self.solveParseTree(treeNode.children[0]), self.solveParseTree(treeNode.children[1]))
        else:
            print "Error when parsing tree, no operations has more than 2 operands"
            return None
        
        
    #generate parse tree, add URLFunction objects into the parse tree, return tree root
    def _parse2Functions(self, functionLstStr):
        trNd = LocationParseTreeNode(functionLstStr)
        self._buildParseTree(trNd, 0)
        self.parseTreeRoot = trNd
        return trNd
    
    def _createNodeIdSet(self):
        if self.locationTreeNode != None:
            self.nodeIdSet = self.locationTreeNode.idSet
    
    #be able to parse sth like near(0,1,2)
    def parseURL(self):
        if self.urlStr == None or len(self.urlStr.rstrip()) == 0:
            self.urlStr = self.locationTree.root.name + u"#" + u"getAll()"
        print self.urlStr
        tmpStrLst = self.urlStr.split(u'#')
        if(len(tmpStrLst)>2):
            print "Error while parsing url:" + self.urlStr +", more than one # found!"
            return False
        if len(tmpStrLst)==1:
            tmpStrLst.append(u"getAll()")
        self.locationTreeNode = self.locationTree.findLocation(self.locationTree.root, tmpStrLst[0])
        if self.locationTreeNode == None:
            print "Error Location not right"
            return False
        self._parse2Functions(tmpStrLst[1])
        self._createNodeIdSet()
        return True
    
    
class URLFunction:
    def __init__(self, name, func, locationTreeNode):
        self.name = name
        self.func = func
        self.args = [locationTreeNode]
    def addParameter(self, para):
        self.args += para
    def runFunction(self):
        return self.func(*self.args)
    def toString(self):
        stri = self.name+"("
        for i in range(len(self.args)-1):
            if len(self.args[i+1])>0:   #there are cases the parameter is an empty string which means no parameter at all
                stri = stri + str(self.args[i+1])+","
        stri = stri + ")"
        return stri
        


if __name__ == "__main__":
    locTree = LocationTree(u"universal")
    loc0 = u"universal/Boli_Building/3F/South_Corridor/Room318@(3,1,8)"
    loc1 = u"universal/Boli_Building/3F/South_Corridor/Room318@(4,1,7)"
    loc2 = u"universal/Boli_Building/3F/South_Corridor/Room318@(4,4,4)"
    loc3 = u"universal/Boli_Building/3F/South_Corridor/Room318@(5,2,2)"
    senNd0 = SensorNode(Node(0, loc0))
    senNd1 = SensorNode(Node(1, loc1))
    senNd2 = SensorNode(Node(2, loc2))
    senNd3 = SensorNode(Node(3, loc3))
    locTree.addSensor(senNd0)
    locTree.addSensor(senNd1)
    locTree.addSensor(senNd2)
    locTree.addSensor(senNd3)
    locTree.printTree(locTree.root, 0)
    query = u"universal/Boli_Building/#closest(4,4,4)"
    locURLHandler = LocationURL(query, locTree)
    locURLHandler.parseURL()
#   print locURLHandler.locationTreeNode.sensorLst
    print locURLHandler.solveParseTree()
    
    

