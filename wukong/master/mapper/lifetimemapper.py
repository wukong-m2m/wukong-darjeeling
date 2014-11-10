# This file defines two mappers that maximize the life time of system in below. One is consider
# the impact of communication distance, and the other don't consider that.

from pulp import *
from base.mapper import *

class DistanceIgnorantEnergyEfficientMapper(AbstractSelectionMapper):

  def __init__(self, application, tree):


class DistanceAwareEnergyEfficientMapper(AbstractSelectionMapper):

  def __init__(self, application, tree):