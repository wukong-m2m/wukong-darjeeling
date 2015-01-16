import os, sys
from configuration import *
from wkpf.model.locationTree import *
from wkpf.model.models import *
from wkpf.wkpfcomm import *


dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

class ModelManager:
    __model_manager = None

    @classmethod
    def init(cls):
        if(cls.__model_manager == None):
            cls.__model_manager = ModelManager()

        return cls.__model_manager

    def __init__(self):
        self._virtual_nodes = {}   # for monitoring
        self._node_infos = {}    # node id to node instance dictionary
        self._class_id_map = {}   # class id to nodes list dictionary
        self._location_tree = LocationTree(LOCATION_ROOT)
        self._wkpfcomm = getComm()
        self.initializeVirtualNode()


    def getNodeInfos(self):
        return self._node_infos.values()

    def getNodeInfo(self, node_id):
        # return getComm().getNodeInfo(node_id)
        return self._node_infos[int(node_id)]

    def getPossibleHostDevice(wuclassId):
        if wuclassId in self._class_id_map:
            return self._class_id_map[wuclassId]
        else:
            return []

    def updateMaps(self):
        self._class_id_map = {}
        for id in self._node_infos:
            node = self._node_infos[id]
            for wuclass in node.wuclasses:
                if not wuclass.id in self._class_id_map:
                    self._class_id_map[wuclass.id] = []
                self._class_id_map[wuclass.id].append[node]

    def refreshNodeInfo(self, condition=True):
        self.clearNodes()
        nodes = self._wkpfcomm.getAllNodeInfos(self._node_infos.values(), condition)
        for node in nodes:
            self._node_infos[node.id] = node
        self._node_infos = dict(self._node_infos.items() + self._virtual_nodes.items())
        self.updateMaps()
        self.saveNodes()
        self.rebuildTree(copy.deepcopy(self._node_infos.values()))

    def getVirtualNodes(self):
        return self._virtual_nodes

    def findLocationById(self, id):
        self._location_tree.findLocationById(id)

    def saveLocationTree(self):
        self._location_tree.saveTree()

    # using cloned nodes
    def rebuildTree(self, nodes_clone):
        self._location_tree = LocationTree(LOCATION_ROOT)
        self._location_tree.buildTree(nodes_clone)
        flag = os.path.exists("../../LocalData/landmarks.txt")
        if(flag):
            self._location_tree.loadTree()
        self._location_tree.printTree()

    def loadTree(self):
        self._location_tree.loadTree()

    def getLocationTreeJson(self):
        return self._location_tree.getJson()

    def getLocationTree(self):
        return self._location_tree

    def addSensor(self, sensor):
        self._location_tree.addSensor(sensor)

    def addLandMark(self, landmark):
        self._location_tree.addLandMark(landmark)

    def deleteLandMark(self, name, location):
        self._location_tree.delLandMark(name, location)

    # add the virtual wunode just used to represent master which is the destination of monitoring link
    def initializeVirtualNode(self):
        # Add the server as a virtual Wudevice for monitoring
        wuclasses = {}
        wuobjects = {}

        # 1 is by default the network id of the controller
        node = WuNode(1, '/' + LOCATION_ROOT, wuclasses, wuobjects, 'virtualdevice')
        wuclassdef = WuObjectFactory.wuclassdefsbyid[44]
        wuobject = WuObjectFactory.createWuObject(wuclassdef, node, 1, False)
        self._virtual_nodes[1] = node

    def dumpNodesXML(self):
        root = ElementTree.Element('Nodes')
        for id, node in self._node_infos.items():
            node_element = ElementTree.SubElement(root, 'Node')
            node_element.attrib['id'] = str(id)
            node_element.attrib['type'] = str(node.type)
            location_element = ElementTree.SubElement(node_element, "Location")
            location_element.attrib['length'] = str(len(node.location))
            location_element.attrib['content'] = str(node.location)

            wuclasslist_element = ElementTree.SubElement(node_element,"WuClassList")
            wuclasslist_element.attrib['length'] = str(len(node.wuclasses))
            for wuclassid, wuclass in  node.wuclasses.items():
                wuclass_element = ElementTree.SubElement(wuclasslist_element,"WuCLass")
                wuclass_element.attrib['id'] = str(wuclassid)
                wuclass_element.attrib['virtual'] = str(wuclass.virtual)

            wuobjectlist_element = ElementTree.SubElement(node_element,"WuObjectList")
            for port, wuobject in  node.wuobjects.items():
                wuobject_element = ElementTree.SubElement(wuobjectlist_element,"WuObject")
                wuobject_element.attrib['id'] = str(wuobject.wuclassdef.id)
                wuobject_element.attrib['virtual'] = str(wuobject.virtual)
                wuobject_element.attrib['port'] = str(port)
            # for prop_name, prop_value in wuobject.properties.items():
            #     wuproperty_element = ElementTree.SubElement(wuobject_element,"WuProperty")
            #     wuproperty_element.attrib['name'] = str(prop_name)
            #     wuproperty_element.attrib['value'] = str(prop_value)

        #for more human readable xml
        rough_stri = ElementTree.tostring(root, 'utf-8')
        xml_content = xml.dom.minidom.parseString(rough_stri)
        pretty_stri = xml_content.toprettyxml()
        return pretty_stri

    def findById(self, id):
        if id in self._node_id_map.keys():
            return self._node_id_map[id]
        return None


    def saveNodes(self, filename="LocalData/nodes.xml"):#for debug now, will expand to support reconstructing nodes from the dump ---- Sen

        fin = open(os.path.join(dir, filename),"w")
        fin.write(self.dumpNodesXML())
        fin.close()
        return

    def clearNodes(self, filename="LocalData/nodes.xml"):
        self.node_dict = {}
        fin = open(os.path.join(dir, filename),"w")
        fin.write("")
        fin.close()
        return

    def loadNodes(self, filename="LocalData/nodes.xml"):#for debug now, will expand to support reconstructing nodes from the dump ---- Sen
        print ('[loadNodes in models] Loading node from file', os.path.join(dir, filename))
        try:
            fin = open(os.path.abspath(os.path.join(dir, filename)),"r")
            nodedom = xml.dom.minidom.parse(fin)
        except Exception as e:
            print (filename,'does not exist, initial list is empty!')
            return self._node_id_map.values()

        nodes = nodedom.getElementsByTagName("Node")
        for node_ele in nodes:
            nodeid = int(node_ele.getAttribute("id"))
            nodetype = node_ele.getAttribute("type")
            wuclasses = {}
            wuobjects = {}
            location = ''
            node = WuNode(nodeid, location, wuclasses, wuobjects,type=nodetype) #note: wuclasses, pass by reference, change in original list is also change in node
            if node_ele.hasChildNodes():
                for prop_ele in node_ele.childNodes:
                    if prop_ele.nodeType != prop_ele.ELEMENT_NODE:
                        continue
                    if prop_ele.tagName == "Location":
                        node.location = prop_ele.getAttribute("content")
		            # WuNode.locations[nodeid] = node.location
                    if prop_ele.tagName == "WuClassList":
                        for wuclass_ele in prop_ele.childNodes:
                            if wuclass_ele.nodeType != wuclass_ele.ELEMENT_NODE:
                                continue
                            wuclass_id = int(wuclass_ele.getAttribute("id"), 0)
                            virtual = True if wuclass_ele.getAttribute("virtual").lower()=="true" else False
                            try:
                                wuclassdef = WuObjectFactory.wuclassdefsbyid[wuclass_id]

                            except KeyError:
                                print '[loadNodes in models] Unknown wuclass id', wuclass_id
                                print ('[loadNodes in models] Invalid saved discover input file', filename)
                                return

                            wuclasses[wuclass_id] = wuclassdef
                    if prop_ele.tagName == "WuObjectList":
                        for wuobj in prop_ele.childNodes:
                            if wuobj.nodeType != wuobj.ELEMENT_NODE:
                                continue
                            port_number = int(wuobj.getAttribute("port"), 0)
                            wuclass_id = int(wuobj.getAttribute("id"), 0)
                            virtual = True if wuobj.getAttribute("virtual").lower()=="true" else False
                            try:
                                wuclassdef = WuObjectFactory.wuclassdefsbyid[wuclass_id]
                            except KeyError:
                                print '[loadNodes in models] Unknown wuclass id', wuclass_id
                                print ('[loadNodes in models] Invalid saved discover input file', filename)
                                return
                            wuobject = WuObjectFactory.createWuObject(wuclassdef, node, port_number, virtual)


                    # for wuprop in wuobj.childNodes:
                    #     if wuobj.nodeType != wuobj.ELEMENT_NODE:
                    #         continue
                    #     prop_name = wuprop.getAttribute("name")
                    #     prop_value = int(wuprop.getAttribute("value"),0)
                    #     wuobject.properties[prop_name] = prop_value

        #add the virtual nodes
        self._node_infos = dict(self._node_infos.items() + self._virtual_nodes.items())
        return self._node_infos.values()


