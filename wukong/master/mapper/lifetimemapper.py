"""
This file defines the mapper that maximize the life time of system in below. It is the algorithm for MEDES 2014
paper  which consider the energy consumption of transmission distance.

It uses rule to reduce Mixed Integer None-Linear Programming Problem to Integer Linear Progressming problem.

Assume we have N classes in a FBP, and M devices in Wukong System. A link l_i_j of a FBP can map to a set H<i, j>
of pair of devices p<dn, dm> within the system. Then we can define the optimization problem as below:


x_i_n * x_j_m => y_i_n_j_m

min(Segma_ij(Segma_H(i,j)(x_i_n * x_j_m * w_n_m)) 1 <= i <= N; 1 <= j <= N; 1 <= n <= M; 1 <= m <= M
"""
from pulp import *
from base.mapper import *


class DistanceAwareEnergyEfficientMapper(AbstractSelectionMapper):

    def __init__(self, application, modelmanager):
        super().__init__(application, modelmanager)

    """The entry point for setting IP constraints for a mapping algorithm """
    def _build_problem():
        prob  = LpProblem("Maximize Lifetime Problem", LpMinimize)


    """Apply energy constraints on device to the ILP problem."""
    def _apply_wudevice_energy_constraints(problem):
      pass

    """Apply constraints from transforming mix-max to min problem."""
    def _apply_upper_bound_constraints(problem):
      pass

    """Apply location based constraints to the ILP problem."""
    def _apply_location_constraints(problem):
      pass

    """Apply wuclass constraints(problem)
    def _apply_wuclass_constraints(problem):
      pass

    """Apply selection result into Wukong System
    def _apply_result():