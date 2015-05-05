/*
 * panic.h
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
 

#ifndef __panic_h
#define __panic_h

#include "types.h"
#include "core.h"
 
#define DJ_PANIC_OUT_OF_MEMORY              RUNLEVEL_PANIC+0
#define DJ_PANIC_ILLEGAL_INTERNAL_STATE     RUNLEVEL_PANIC+1
#define DJ_PANIC_UNIMPLEMENTED_FEATURE      RUNLEVEL_PANIC+2
#define DJ_PANIC_UNCAUGHT_EXCEPTION         RUNLEVEL_PANIC+3
#define DJ_PANIC_UNSATISFIED_LINK			RUNLEVEL_PANIC+4
#define DJ_PANIC_MALFORMED_INFUSION			RUNLEVEL_PANIC+5
#define DJ_PANIC_ASSERTION_FAILURE			RUNLEVEL_PANIC+6
#define DJ_PANIC_SAFE_POINTER_OVERFLOW		RUNLEVEL_PANIC+7
#define DJ_PANIC_INFUSION_VERSION_MISMATCH	RUNLEVEL_PANIC+8
#define DJ_PANIC_UNSUPPORTED_OPCODE			RUNLEVEL_PANIC+9
// Reserved 100-109 for wkcomm
// Reserved 110-119 for wkpf
// Reserved 120-129 for eco

void dj_panic(int32_t panictype);

#endif
