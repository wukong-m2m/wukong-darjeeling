class VirtualNodeProperties:
  def __init__(self, properties=[]):
    self.properties = properties

  def __iter__(self):
    for elem in self.properties:
      yield elem

  def getProperty(self, name):
    for property in self.properties:
      if property.name == name:
        return property

  def get(self, name):
    for property in self.properties:
      if property.name == name:
        return property.get()

  def set(self, name, value):
    for property in self.properties:
      if property.name == name:
        return property.set(value)

class VirtualNodeProperty:
  def __init__(self, name, number):
    self.number = number # the number from WuPropertyDef in models.py
    self.name = name
    self.dirty = False
    self.value = None

  # FIXME: will have to add type checking
  def set(self, value):
    self.dirty = True
    self.value = value

  def get(self):
    return self.value

# The base class for user programmed python wuclass
class VirtualNodeWuClass:
  def __init__(self, port_number, node_id, properties=VirtualNodeProperties()):
    self.port_number = port_number
    self.node_id = node_id
    self.properties = properties

