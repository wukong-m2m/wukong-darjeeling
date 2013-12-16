from wuclass_helper import *
class Dimmer(VirtualNodeWuClass):
  def setup(self):
    pass

  def update(self, virtualnode, properties, links):
    print 'dimmer update'
    comm = virtualnode.comm
    on_off = properties.get("on_off")
    level = properties.get("level")
    node_id = self.node_id
    print 'dimmer on/off', on_off
    print 'dimmer level', level
    if on_off:
      if self.port_number == 70:
    	print 'setting node', node_id, 'level', level
        comm.zwave.send_raw(node_id, [0x20, 0x01, level,])
      elif self.port_number == 71:
        comm.zwave.send_raw(node_id, [0x60,0x0d,0x1,0x2,0x20, 0x01, level,])
      elif self.port_number == 72:
        comm.zwave.send_raw(node_id, [0x60,0x0d,0x1,0x3,0x20, 0x01, level,])
