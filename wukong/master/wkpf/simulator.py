from globals import *
import xml.dom.minidom
from models import *

class MockDiscovery:
    def new_message(*args):
        return Message(*args)
    def __init__(self):
        self.dom = xml.dom.minidom.parse(MOCK_XML);
    def discovery(self):
        nodes = self.dom.getElementsByTagName("Node")
        nodeList = []
        for node in nodes:
            nodeList.append(node.getAttribute("id"))
        return nodeList
    def mockLocation(self, nodeId):
        nodes = self.dom.getElementsByTagName("Node")
        location = ''
        for node in nodes:
            if int(node.getAttribute("id"),0) == nodeId:
                if node.hasChildNodes():
                    for lsts in node.childNodes:
                        if lsts.nodeType != lsts.ELEMENT_NODE:    
                            continue
                        if lsts.tagName == "Location":
                            location = lsts.getAttribute("content")
                            return location
        return location
        
    def mockWuClassList(self, nodeId):
        wuclasses = []
        nodes = self.dom.getElementsByTagName("Node")
        found = False;
        for node in nodes:
            if int(node.getAttribute("id"),0) == nodeId:
                if node.hasChildNodes():
                    for lsts in node.childNodes:
                        if lsts.nodeType != lsts.ELEMENT_NODE:    
                            continue
                        if lsts.tagName == "WuClassList":
                            for wuclass in lsts.childNodes:
                                if wuclass.nodeType != wuclass.ELEMENT_NODE:    
                                    continue
                                wuclass_id = int(wuclass.getAttribute("id"), 0)
                                publish = wuclass.getAttribute("publish")
                                virtual = True if wuclass.getAttribute("virtual")=="true" else False
                                if publish == "true":
                                    node = WuNode.find(id=nodeId)
                                    if not node:
                                      print '[wkpfcomm] Unknown node id', nodeId
                                      break

                                    wuclassdef = WuClassDef.find(id=wuclass_id)

                                    if not wuclassdef:
                                      print '[wkpfcomm] Unknown wuclass id', wuclass_id
                                      break

                                    wuclass = WuClass.find(node_identity=node.identity,
                                        wuclassdef_identity=wuclassdef.identity)

                                    if not wuclass:
                                      wuclass = WuClass.create(wuclassdef, node, virtual)

                                    wuclasses.append(wuclass)
                            found =  True;
                            break
                        if found:
                            break
            if found:
                break
        return wuclasses
    def mockWuObjectList(self, nodeId):
        wuobjects = []
        nodes = self.dom.getElementsByTagName("Node")
        found = False;
        for node in nodes:
            if int(node.getAttribute("id"),0) == nodeId:
                if node.hasChildNodes():
                    for lsts in node.childNodes:
                        if lsts.nodeType != lsts.ELEMENT_NODE:    
                            continue
                        if lsts.tagName == "WuObjectList":
                            for wuobj in lsts.childNodes:
                                if wuobj.nodeType != wuobj.ELEMENT_NODE:    
                                    continue
                                port_number = int(wuobj.getAttribute("port"), 0)
                                wuclass_id = int(wuobj.getAttribute("id"), 0)
                                node = WuNode.find(id=nodeId)
                                if not node:
                                  print '[wkpfcomm] Unknown node id', nodeId
                                wuclassdef = WuClassDef.find(id=wuclass_id)
                                if not wuclassdef:
                                  print '[wkpfcomm] Unknown wuclass id', wuclass_id
                                wuobject = WuObject.find(node_identity=node.identity,
                                    wuclassdef_identity=wuclassdef.identity)
                                if not wuobject:
                                  wuobject = WuObject.create(wuclassdef, node, port_number)
                                wuobjects.append(wuobject)
                            found =  True;
                            break
                        if found:
                            break
            if found:
                break
        return wuobjects
        
                                
