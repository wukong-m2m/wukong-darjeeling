from configuration import *
from wkpf.model.locationTree import *
from wkpf.model.models import *
from wkpf.wkpfcomm import *

class ModelManager:
    __model_manager = None

    @classmethod
    def init(cls):
        if(cls.__model_manager == None):
            cls.__model_manager = ModelManager()

        return cls.__model_manager

    def __init__(self):
        self._virtual_nodes = {}   # for monitoring
        self._node_infos = []
        self._node_id_map = {}    # node id to node instance dictionary
        self._class_id_map = {}   # class id to nodes list dictionary
        self._location_tree = LocationTree(LOCATION_ROOT)
        self._wkpfcomm = getComm()
        self.initializeVirtualNode()


    def getNodeInfos(self):
        return self._node_infos

    def getNodeInfo(self, node_id):
        return getComm().getNodeInfo(node_id)

    def refreshNodeInfo(self, condition=True):
        # WuNode.clearNodes()
        self._node_infos = self._wkpfcomm.getAllNodeInfos(condition)
        for index in self._virtual_nodes:
            self._node_infos.append(self._virtual_nodes[index])
        WuNode.addNodes(self._node_infos)
        WuNode.saveNodes()
        self.rebuildTree(copy.deepcopy(self._node_infos))

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


