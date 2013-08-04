from base import Definition

class WuLink(Definition):
  def __init__(self, from_component_index, from_property_id,
          to_component_index, to_property_id, to_wuclass_id):
    self.from_component_index = from_component_index
    self.from_property_id = from_property_id
    self.to_component_index = to_component_index
    self.to_property_id = to_property_id
    self.to_wuclass_id = to_wuclass_id
