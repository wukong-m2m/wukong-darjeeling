from globals import *
import xml.dom.minidom
from model.models import *

class MockDiscovery:
    def new_message(*args):
        return Message(*args)
    def __init__(self):
        self.dom = xml.dom.minidom.parse(MOCK_XML);
    def discovery(self):
        self.dom = xml.dom.minidom.parse(MOCK_XML);
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
        wuclasses = {}
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
                                    wunode = WuNode.findById(nodeId)
                                    try:
                                      wuclassdef = WuObjectFactory.wuclassdefsbyid[wuclass_id]

                                    except KeyError:
                                      print '[wkpfcomm] Unknown wuclass id', wuclass_id
                                      break

                                    wuclasses[wuclass_id] = wuclassdef
                            found =  True;
                            break
                        if found:
                            break
            if found:
                break
        return wuclasses
    def mockWuObjectList(self, nodeId):
        wuobjects = {}
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
                                virtual = False
                                wunode = WuNode.findById(nodeId)
                                try:
                                  wuclassdef = WuObjectFactory.wuclassdefsbyid[wuclass_id]
                                except KeyError:
                                  print '[wkpfcomm] Unknown wuclass id', wuclass_id
                                  break
                                if (not wunode) or (port_number not in wunode.wuobjects.keys()) or wunode.wuobjects[port_number].wuclassdef != wuclassdef:
                                  wuobject = WuObjectFactory.createWuObject(wuclassdef, wunode, port_number, virtual)
                                else:
                                  wuobject = wunode.wuobjects[port_number]
                                wuobjects[port_number] = wuobject
                            found =  True;
                            break
                        if found:
                            break
            if found:
                break
        return wuobjects
        
                                
