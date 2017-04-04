#ifndef CORE_OPTIMISATIONS_H
#define CORE_OPTIMISATIONS_H

#define CORE_OPTIMISATION_SHORT_ARRAY_INDEX
#define CORE_OPTIMISATION_CALC_I_TIMES_N_OUTSIDE_LOOP

// short array index for matrix benchmark
// store i*N in variable to use in inner loop
// Refactor CoreState to do less GETFIELD_As
// New GETFIELD_A_FIXED when offset is known
// Optimise crc8
// Optimise core_bench_matrix
// SIMUL 16x16->32 multiplication opcode
// Optimise core_bench_matrix
// Fix in method call & optimise TABLESWITCH
// Optimise popping and flushing before CALLs
// Small optimisation in CoreState
// cheat to avoid NEWARRAY
// cheat to avoid INVOKEVIRTUAL
// cheat by isolating core_state_transition call
// cheat by force inlining ee_isdigit
// cheat by refactoring crc to reduce funcalls


#endif // CORE_OPTIMISATIONS_H