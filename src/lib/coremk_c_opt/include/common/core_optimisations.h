#ifndef CORE_OPTIMISATIONS_H
#define CORE_OPTIMISATIONS_H

// short array index for matrix benchmark
#define CORE_OPTIMISATION_SHORT_ARRAY_INDEX

// store i*N in variable to use in inner loop
#define CORE_OPTIMISATION_CALC_I_TIMES_N_OUTSIDE_LOOP

// Refactor CoreState to do less GETFIELD_As
	// ! Seems there's no C equivalent for this

// Optimise crc8
#define CORE_OPTIMISATION_OPTIMISE_CRC_1

// Optimise core_bench_matrix
#define CORE_OPTIMISATION_MANUALLY_INLINE_BIT_EXTRACT

// Optimise core_bench_matrix
#define CORE_OPTIMISATION_AVOID_SOME_ADDITIONS  // replaces CORE_OPTIMISATION_CALC_I_TIMES_N_OUTSIDE_LOOP
#define CORE_OPTIMISATION_REDUCE_C_ARRAY_ACCESS // replaces CORE_OPTIMISATION_CALC_I_TIMES_N_OUTSIDE_LOOP

// Small optimisation in CoreState
#define CORE_OPTIMISATION_SMALL_CORE_STATE_OPTIMISATION

// cheat to avoid NEWARRAY
#define CORE_OPTIMISATION_AVOID_NEWARRAY

// cheat to avoid INVOKEVIRTUAL
#define CORE_OPTIMISATION_AVOID_INVOKEVIRTUAL

// cheat by isolating core_state_transition call
#define CORE_OPTIMISATION_ISOLATE_CALL_TO_CORE_STATE_TRANSITION

// cheat by force inlining ee_isdigit
	// ! Already inlined by avr-gcc.

// cheat by refactoring crc to reduce funcalls
#define CORE_OPTIMISATION_REDUCE_CRC_FUNCALLS

// cheat to avoid NEW
#define CORE_OPTIMISATION_AVOID_NEW

#endif // CORE_OPTIMISATIONS_H