class Dimmer(VirtualNodeWuClass):
  def setup(self, comm):
    self.comm = comm

  def update(self, properties, links):
    on_off = properties.get("on_off")
    level = properties.get("level")
    node_id = self.node_id
    if on_off is True:
      if self.port_number == 70:
        reply = self.comm.zwave.send_raw(node_id, [0x20, 0x01, level,])
      elif self.port_number == 71:
        reply = self.comm.zwave.send_raw(node_id, [0x60,0x0d,0x1,0x2,0x20, 0x01, level,])
      elif self.port_number == 72:
        reply = self.comm.zwave.send_raw(node_id, [0x60,0x0d,0x1,0x3,0x20, 0x01, level,])
