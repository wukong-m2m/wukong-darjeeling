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

#include "config.h"
#include "types.h"
#include "core.h"
 
// RUNLEVEL_PANIC==3
#define DJ_PANIC_OUT_OF_MEMORY                            RUNLEVEL_PANIC+0
#define DJ_PANIC_UNIMPLEMENTED_FEATURE                    RUNLEVEL_PANIC+2
#define DJ_PANIC_UNCAUGHT_EXCEPTION                       RUNLEVEL_PANIC+3
#define DJ_PANIC_UNSATISFIED_LINK			              RUNLEVEL_PANIC+4
#define DJ_PANIC_MALFORMED_INFUSION			              RUNLEVEL_PANIC+5
#define DJ_PANIC_ASSERTION_FAILURE			              RUNLEVEL_PANIC+6
#define DJ_PANIC_SAFE_POINTER_OVERFLOW		              RUNLEVEL_PANIC+7
#define DJ_PANIC_INFUSION_VERSION_MISMATCH	              RUNLEVEL_PANIC+8
#define DJ_PANIC_UNSUPPORTED_OPCODE			              RUNLEVEL_PANIC+9
#define DJ_PANIC_REPROGRAM_OUTSIDE_REGION	              RUNLEVEL_PANIC+10
#define DJ_PANIC_OFFSET_TOO_LARGE                         RUNLEVEL_PANIC+11
#define DJ_PANIC_AOT_STACKCACHE_IN_USE                    RUNLEVEL_PANIC+12
#define DJ_PANIC_AOT_STACKCACHE_NOTHING_TO_SPILL          RUNLEVEL_PANIC+13
#define DJ_PANIC_AOT_STACKCACHE_PUSHED_REG_NOT_IN_USE     RUNLEVEL_PANIC+14
#define DJ_PANIC_AOT_STACKCACHE_INVALID_POP_TARGET        RUNLEVEL_PANIC+15
#define DJ_PANIC_AOT_STACKCACHE_NO_SPACE_FOR_POP          RUNLEVEL_PANIC+16
#define DJ_PANIC_AOT_MARKLOOP_LOW_WORD_NOT_FOUND          RUNLEVEL_PANIC+17
#define DJ_PANIC_CHECKCAST_FAILED                         RUNLEVEL_PANIC+18
#define DJ_PANIC_ILLEGAL_INTERNAL_STATE_ARRAY_COMPONENT   RUNLEVEL_PANIC+19
#define DJ_PANIC_ILLEGAL_INTERNAL_STATE_THREAD_FRAME_NULL RUNLEVEL_PANIC+20
#define DJ_PANIC_ILLEGAL_INTERNAL_STATE_NO_RUNTIME_CLASS  RUNLEVEL_PANIC+21
#define DJ_PANIC_AOT_ASM_ERROR                            RUNLEVEL_PANIC+50
// Reserved 100-109 for wkcomm
// Reserved 110-119 for wkpf
// Reserved 120-129 for eco

#ifdef AVRORA
#define dj_panic(panictype)     { avroraPrintPanic(panictype); asm volatile ("break"); }
#else
void dj_panic(uint8_t panictype);
#endif

#endif // __panic_h
