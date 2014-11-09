"""
It controls the status of all the devices, wuobjects, locations, applications that are managed by the master
Any operations on models or location tree should be done within it.
"""

from configuration import *
from wkpf.model import *
from wkpf.wkpfcomm import *
from make_js import make_main
from make_fbp import fbp_main
import ApplicationManager

class SystemManager:
    __system_manager = None

    @classmethod
    def init(cls):
        if(cls.__system_manager == None):
            cls.__system_manager = SystemManager()
        return cls.__system_manager

    def __init__(self):
        self._applications = []
        self._location_tree = LocationTree(LOCATION_ROOT)
        self._wukong_status = []
        self._virtual_nodes = {}   # for monitoring
        self._node_infos = []
        self._connected = (False if SIMULATION == "true" else True)  # whether zwave gateway is connected
        self._busy = False
        self._wkpfcomm = getComm()
        self._appmanager = ApplicationManager(self._applications, self._busy)
        initZwave()  # test zwave module availability
        initMonitoring()  # test mongoDB setting
        updateApplications()
        importWuXML()
        makeFBP()

    def initZwave(self):
        if WKPFCOMM_AGENT == "ZWAVE":
            try:
                import pyzwave
                m = pyzwave.getDeviceType
            except:
                print "Please install the pyzwave module in the wukong/tools/python/pyzwave by using"
                print "cd ../tools/python/pyzwave; sudo python setup.py install"
                sys.exit(-1)

    def initMonitoring(self):
        if(MONITORING == 'true'):

        # check availability of mongo driver
        try:
            from pymongo import MongoClient
        except:
            print "Please install python mongoDB driver pymongo by using"
            print "easy_install pymongo"
            sys.exit(-1)

        # check correctness of mongoDB URL
        try:
            wkpf.globals.mongoDBClient = MongoClient(MONGODB_URL)
        except:
            print "MongoDB instance " + MONGODB_URL + " can't be connected."
            print "Please install the mongDB, pymongo module."
            sys.exit(-1)

    def getApplicationSize(self):
        return len(applications)

    def getActiveApplication(self):
        try:
            return applications[active_ind]
        except:
            return None

    def setActiveApplicationIndex(self, new_index):
        self.active_ind = new_index

    def getLocationTreeJson(self):
        return self._location_tree.getJson()



    def signalDeploy(self, app_ind, platforms):
         # signal deploy in other greenlet task
          wusignal.signal_deploy(platforms)
          self._active_ind  = app_ind

    def getNodeInfos(self):
        return self._node_infos

    def getNodeInfo(self, node_id):
        return getComm().getNodeInfo(node_id)

    def refreshNodeInfo(self, conditio=False):
        self._node_infos = self._wkpfcomm.getAllNodeInfos(condition)
        rebuildTree(self._node_infos)

    def findLocationById(self, id):
        self._location_tree.findLocationById(id)

    def saveLocationTree(sef):
        self._location_tree.saveLocationTree()

    # add the virtual wunode just used to represent master which is the destination of monitoring link
    def initializeVirtualNode(self):
        # Add the server as a virtual Wudevice for monitoring
        wuclasses = {}
        wuobjects = {}

        # 1 is by default the network id of the controller
        node = WuNode(1, '/' + LOCATION_ROOT, wuclasses, wuobjects, 'virtualdevice')
        wuclassdef = WuObjectFactory.wuclassdefsbyid[44]
        wuobject = WuObjectFactory.createWuObject(wuclassdef, node, 1, False)
        wkpf.globals.virtual_nodes[1] = node

    # using cloned nodes
    def rebuildTree(self):
        nodes_clone = copy.deepcopy(_node_infos)
        location_tree = LocationTree(LOCATION_ROOT)
        location_tree.buildTree(nodes_clone)
        flag = os.path.exists("../../LocalData/landmarks.txt")
        if(flag):
            location_tree.loadTree()
        location_tree.printTree()

     def loadTree(self):
        self._location_tree.loadTree()

    # Helper functions
    def setupSignalHandlerGreenlet(self):
        logging.info('setting up signal handler')
        gevent.spawn(wusignal.signal_handler)

    def isConnected(self):
        return connected

    def disconnect(self):
        connected = False

    def isMasterBusy(self):
        return self._busy

    def setMasterBusy(self):
        self._busy = True

    def setMasterAvailable(self):
        self._busy = False;

    def getAllWukongStatus(self):
        return self._wukong_status

    def getWukongStatus(self):
        if len(self._wukong_status) > 0:
            return self._wukong_status[len(self._wukong_status)-1]
        else:
            return ""

    def setWukongStatus(status):
        self._wukong_status.append(status)

    def importWuXML(self):
        make_main()

    def makeFBP():
        test_1 = fbp_main(self)
        test_1.make()

    def getActiveNodeInfos(self, force=False):
        return getComm().getActiveNodeInfos(force)

    def getCurrentCommStatus(self):
        return self._wkpfcomm.currentStatus()