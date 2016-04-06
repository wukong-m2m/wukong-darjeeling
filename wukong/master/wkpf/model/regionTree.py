import sys, traceback, os
sys.path.append(os.path.join(os.path.dirname(__file__), "../.."))

import logging
import re
from models import *

region_path_pattern = r'\/([^/]+\/)*'

class RegionNode:
    def __init__(self, path, parent):
        self.path = path
        self.parent = parent
        self.isRoot = (parent == None)
        self.children = {}
        self.devices = []
        if self.isRoot:
            print 'create root node:' + path
        else:
            print 'create node:' + path

    def isRoot():
        return self.isRoot

    def addDevice(self, device):
        self.devices.append(device)

    def delDevice(self, device):
        self.devices.remove(device)

    def getDevices(self):
        return self.devices

    def addChild(self, path, node):
        self.children[path] = node;

    def delChild(self, path):
        del self.children[path]

    def getRegionPath(self):
        node = self
        regionPath = ''
        while (node != None and not node.isRoot):
            regionPath = node.path + '/' + regionPath
            node = node.parent
        return '/' + regionPath

    def findRegionNode(self, pathList):
        if pathList == []:
            return self
        else:
            if self.children.get(pathList[0]) != None:
                child = self.children[pathList[0]]
                del pathList[0]
                return child.findRegionNode(pathList);
            else:
                return None

    def findOrCreateRegionNode(self, pathList):
        if pathList == []:
            return self
        else:
            node = None
            if self.children.get(pathList[0]) != None:
                node = self.children[pathList[0]]
            else:
                node = RegionNode(pathList[0], self)
                self.children[pathList[0]] = node;
            del pathList[0]
            return node.findOrCreateRegionNode(pathList)

class RegionTree:
    _pattern = re.compile(region_path_pattern, re.IGNORECASE)
    def __init__(self):
        self.root = RegionNode('', None)

    def _split(self, path):
        pathList = path.split("/")
        del pathList[0]
        if pathList[len(pathList) - 1] == '':
            del pathList[len(pathList) - 1]
        return pathList

    def addRegion(self, regionPath):
        if self._pattern.match(regionPath) != None:
            region = self.root.findOrCreateRegionNode(self._split(regionPath));
            return True
        return False

    def search(self, regionPath):
        if self._pattern.match(regionPath) != None:
            return self.root.findRegionNode(self._split(regionPath))
        else:
            return None

    def addDevice(self, regionPath, device):
        if self._pattern.match(regionPath) != None:
            region = self.root.findOrCreateRegionNode(self._split(regionPath));
            region.addDevice(device);
            return True
        else:
            return False


if __name__ == "__main__":
    path = '/WuKong/EECS/'
    cpe = "/WuKong/EECS/CPE"
    power = "/WuKong/EECS/Power"
    tree = RegionTree()
    tree.addDevice(path, 1)
    tree.addDevice(path, 2)
    node = tree.search(path)
    print node.getRegionPath()
    print node.getDevices()
    tree.addRegion(cpe)
    tree.addRegion(power)
    node = tree.search(path)
    tree.addDevice(cpe, 2)
    tree.addDevice(power, 3)
    print node.children
    node = tree.search(cpe)
    print node.getRegionPath()
    print node.getDevices()
