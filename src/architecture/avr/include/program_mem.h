/*
 * program_mem.h
 * 
 * Copyright (c) 2008-2010 CSIRO, Delft University of Technology.
 * 
 * This file is part of Darjeeling.
 * 
 * Darjeeling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Darjeeling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Darjeeling.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#ifndef __program_mem_h
#define __program_mem_h

#include <avr/pgmspace.h>
#include <stdint.h>

// Macro to put things in program memory
#define DJ_PROGMEM __flash

typedef unsigned int dj_di_pointer;

extern char DJ_SYSTEM_INFUSION_NAME[];

#define DJ_DI_NOT_SET -1
#define DJ_DI_NOT_FOUND -2

#define dj_di_getU8(pointer)  (pgm_read_byte_far(pointer))
#define dj_di_getU16(pointer) (pgm_read_word_far(pointer))
#define dj_di_getU32(pointer) (pgm_read_dword_far(pointer))
#define dj_di_getLocalId(pointer) ((dj_local_id){pgm_read_byte_far(pointer),pgm_read_byte_far(pointer+1)})


// Taken from morepgmspace.h
// http://savannah.nongnu.org/patch/?6352
#define GET_FAR_ADDRESS(var)                          \
({                                                    \
	uint_farptr_t tmp;                                \
                                                      \
	__asm__ __volatile__(                             \
                                                      \
			"ldi	%A0, lo8(%1)"           "\n\t"    \
			"ldi	%B0, hi8(%1)"           "\n\t"    \
			"ldi	%C0, hh8(%1)"           "\n\t"    \
			"clr	%D0"                    "\n\t"    \
		:                                             \
			"=d" (tmp)                                \
		:                                             \
			"p"  (&(var))                             \
	);                                                \
	tmp;                                              \
})

#endif
