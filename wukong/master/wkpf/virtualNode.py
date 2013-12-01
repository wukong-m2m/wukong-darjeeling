import gevent
from models import *

class VirtualNode:
  @classmethod
  def getWuClassDefs(cls):
    return [WuClassDef.find(id=434)]

  _virtualNode = None
  @classmethod
  def init(cls):
    if not cls._virtualNode:
      cls._virtualNode = VirtualNode()
    return cls._virtualNode

  def __init__(self):
    self.changesets = None
    gevent.spawn(self.run)
  def setComm(self,comm):
    self.comm = comm

  def deploy(self, changesets):
    self.changesets = changesets

  def run(self):
    while 1:
      if self.changesets:
        indexes = []
        for ind, component in enumerate(self.changesets.components):
          component_wuclass = WuClassDef.find(name=component.type)
          if component_wuclass.id in [x.id for x in VirtualNode.getWuClassDefs()]:
            indexes.append(ind)

        for link in [link for link in self.changesets.links if link.from_component_index in indexes]:
          self.setProperty(link.to_property_id, self.changesets.components[link.to_component_index], 0)
          gevent.sleep(1) # sleep for 1 sec
          self.setProperty(link.to_property_id, self.changesets.components[link.to_component_index], 255)
          gevent.sleep(1) # sleep for 1 sec
      else:
        gevent.sleep(1) # sleep for 1 sec


  def setProperty(self, to_property_id, component, value):
    wuclassdef = WuClassDef.find(name=component.type)
    wuobject = WuObject.find(wuclassdef_identity=wuclassdef.identity)
    node = WuNode.find(identity=wuobject.node_identity)
    if node.type == 'wudevice':
      pass
    elif node.type == 'native':
      reply = self.comm.zwave.send_raw(node.id, [0x20, 0x01, value,])
      #reply = self.comm.zwave.send_raw(node.id, [0x60,0x0d,0x1,0x2,0x20, 0x01, value,])
      #reply = self.comm.zwave.send_raw(node.id, [0x60,0x0d,0x1,0x3,0x20, 0x01, value,])
