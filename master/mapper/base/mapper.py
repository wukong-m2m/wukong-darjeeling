# In this file, we define the object oriented definition of mapper interface and several abstract mapper class,
# so that when we want to define specific mapping strategy like energy efficiency, prolong system lifetime, QoS maximization,
# Fault tolerant and etc, we can easily pick up a mapping model (greedy or ILP based) to implement it.
#
# A mapper accepts three parameters Wuapplication, WukongSystem, LocationTree as Parameters. After mapping, status of
# the application will be set to be mapped, and mapping results will be recorded in the application also.
#
# Since a mapper is able to update the status of application and system, we need to synchronize every mapping operation in
# AppManager.map()

from pulp import *

class IMapper(Interface):
  """An object that map components of application to wuobjects"""
  def map():


class AbstractMapper(IMapper):

  def __init__(self, application, system, tree):
    self.application = application
    self.system = system
    self.tree = tree

  # To be override
  def map():
    pass


class AbstractSelectionMapper(AbstractMapper):

  def __init__(self, application, system, tree):
    super(self.__class__, self).__init__(application, system, tree)
    self.variables = {}

  def map():
    if (system == None || application == None):
      print "error mapping input"
      return

    problem = _build_problem()
    status = problem.solve(GLPK(msg = 0))
    print LPStatus[status]
    _apply_result()


  """The entry point for setting IP constraints for a mapping algorithm """
  def _build_problem():

  """Apply energy constraints on device to the ILP problem."""
  def _apply_wudevice_energy_constraints(problem):

  """Apply constraints from transforming mix-max to min problem."""
  def _apply_upper_bound_constraints(problem):


  """Apply location based constraints to the ILP problem."""
  def _apply_location_constraints(problem):
    pass

  """Apply wuclass constraints(problem)
  def _apply_wuclass_constraints(problem):
    pass

  """Apply selection result into Wukong System
  def _apply_result():


class AbstractGreedyMapper(AbstractMapper):