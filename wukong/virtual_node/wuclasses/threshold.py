from wuclass_helper import *

LT=0
GT=1
LTE = 2
GTE = 3

class Threshold(VirtualNodeWuClass): 
  def setup(self):
    pass

  def update(self, properties, links):
    threshold = properties.get("threshold")
    operator = properties.get("operator")
    value = properties.get("value")

    '''
    if(((operator == GT && operator == GTE)) and value > threshold) or (((operator == LT && operator == LTE)) and value < threshold) or (((operator == GTE && operator == LTE)) and value == threshold):
      properties.set("output", True)
    else:
      properties.set("output", False)
    '''
