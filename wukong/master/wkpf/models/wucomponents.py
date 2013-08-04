from base import Definition

class WuComponent(Definition):
  def __init__(self, component_index, location, group_size, reaction_time,
          type, application_hashed_name, properties_with_default_values=[]):
    self.index = component_index
    self.location = location
    self.group_size = group_size # int
    self.reaction_time = reaction_time # float
    self.type = type # wuclass name
    self.application_hashed_name = application_hashed_name
    self.properties_with_default_values = properties_with_default_values

    self.instances = [] # WuObjects allocated on various Nodes after mapping
    self.heartbeatgroups = []
