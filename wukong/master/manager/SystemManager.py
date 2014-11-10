"""
It controls the status of all the devices, wuobjects, locations, applications that are managed by the master
Any operations on models or location tree should be done within it.
"""

from configuration import *
from wkpf.model import *
from wkpf.wkpfcomm import *
from make_js import make_main
from make_fbp import fbp_main
from ApplicationManager import ApplicationManager
from ModelManager import ModelManager

class SystemManager:
    __system_manager = None

    @classmethod
    def init(cls):
        if(cls.__system_manager == None):
            cls.__system_manager = SystemManager()
        return cls.__system_manager

    def __init__(self):
        self._wukong_status = []
        self._connected = (False if SIMULATION == "true" else True)  # whether zwave gateway is connected
        self._busy = False
        self._wkpfcomm = getComm()
        self._appmanager = ApplicationManager.init()
        self._modelmanager =  ModelManager.init()
        self.initZwave()  # test zwave module availability
        self.initMonitoring()  # test mongoDB setting
        self._appmanager.updateApplications()
        self.importWuXML()
        self.makeFBP()

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
                self._mongoDBClient = MongoClient(MONGODB_URL)
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

    def signalDeploy(self, app_ind, platforms):
         # signal deploy in other greenlet task
          wusignal.signal_deploy(platforms)
          self._active_ind  = app_ind

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

    def setWukongStatus(self, status):
        self._wukong_status.append(status)

    def importWuXML(self):
        make_main()

    def makeFBP(self):
        test_1 = fbp_main()
        test_1.make()

    def getActiveNodeInfos(self, force=False):
        return getComm().getActiveNodeInfos(force)

    def getCurrentCommStatus(self):
        return self._wkpfcomm.currentStatus()