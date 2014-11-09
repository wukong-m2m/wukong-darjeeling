from configuration import *
from wkpf.model.locationTree import *
from wkpf.model.models import *

class ModelManager:
    __model_manager = None

    @classmethod
    def init(cls):
        if(cls.__model_manager == None):
            cls.__model_manager = ModelManager()

        return cls.__model_manager

    def __init__(self):
        self._nodes = []
        self._node_id_map = {}    # node id to node instance dictionary
        self._class_id_map = {}   # class id to nodes list dictionary
        self._location_tree = LocationTree(LOCATION_ROOT)


    def getNodeInfos(self):
        return self._node_infos

    def getNodeInfo(self, node_id):
        return getComm().getNodeInfo(node_id)

    def refreshNodeInfo(self, condition=False):
        self._node_infos = self._wkpfcomm.getAllNodeInfos(condition)
        self.rebuildTree(copy.deepcopy(self._node_infos))


    def findLocationById(self, id):
        self._location_tree.findLocationById(id)

    def saveLocationTree(sef):
        self._location_tree.saveLocationTree()

    # using cloned nodes
    def rebuildTree(self, nodes_clone):
        location_tree = LocationTree(LOCATION_ROOT)
        location_tree.buildTree(nodes_clone)
        flag = os.path.exists("../../LocalData/landmarks.txt")
        if(flag):
            location_tree.loadTree()
        location_tree.printTree()

    def loadTree(self):
        self._location_tree.loadTree()

    def getLocationTreeJson(self):
        return self._location_tree.getJson()

    def getLocationTree(self):
        return self._location_tree

    def addLandMark(self, landmark):
        self._location_tree.addLandMark(landmark)

    def deleteLandMark(self, name, location):
        self._location_tree.delLandMark(name, location)

