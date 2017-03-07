import os, sys
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
from wkpf.models import *
from wkpf.locationTree import *
from wkpf.wuclasslibraryparser import *
from configuration import *

# these two will be generated
simNodeInfos = []
routing = {}

# load our wuclasses
print "this is test_env"
if os.path.exists('standardlibrary.db'):
  os.remove('standardlibrary.db') 
WuClassLibraryParser.read(COMPONENTXML_PATH) # parse standard library XML and load into db
locs = [u"WuKong", u"WuKong", u"WuKong", u"WuKong"]
locs = [u"/universal/CS_Building/3F/South_Corridor/Room336@(0,1,2)", u"/universal/CS_Building/3F/East_Corridor/Room318@(0,5,4)", u"/universal/CS_Building/3F/South_Corridor/Room336@(0,1,3)", u"/universal/CS_Building/3F/East_Corridor/Room318@(0,6,4)"]
coords = [(0, 1, 2), (0, 5, 3), (0,1,3), (0, 6, 4)]
landmark1 = LandmarkNode(0, u'door',u"/Eng_Hall@(1,0,2)", (2,0.5,4))
landmark2 = LandmarkNode(0, u'window',u"/Eng_Hall@(3,7,5)", (4,0.5,7))

#locs = ["Boli_Building/3F/South_Corridor/Room336", "Boli_Building/3F/East_Corridor/Room318"]
#coords = [(0, 1, 2), (0, 5, 3)]

# generate nodes
for node_id in range(4):
  wuclasses = WuClass.all()
  for port_number, wuclass in enumerate(wuclasses):
    new_wuclass = WuClass(wuclass.id, wuclass.name, wuclass.virtual,
        wuclass.type, [], node_id)
    new_wuclass.save()
    wuobjects = WuObject(node_id, port_number, new_wuclass)
  simNodeInfos.append(
      Node(node_id,
        locs[node_id],
        wuclasses,
        wuobjects
      )
  )
    
# generate routing
ids = [nodeinfo.id for nodeinfo in simNodeInfos]
for id in ids:
  neighbors = ids[:]
  neighbors.remove(id)
  routing[id] = neighbors


# generate location tree
locTree = LocationTree(LOCATION_ROOT)
for node_info, in zip(simNodeInfos):
    locTree.addSensor(SensorNode(node_info))

#queries = ["Boli_Building/3F/South_Corridor/Room318#near(0,1,2,1)|near(1,1,3,1)",
          #"Boli_Building/3F/South_Corridor/Room336#near(0,1,2,1)|near(1,1,3,1)",
          #None, None]
