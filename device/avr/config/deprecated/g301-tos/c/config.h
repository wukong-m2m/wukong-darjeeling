/*
 * config.h
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
 
#ifndef __config_h
#define __config_h

// define heap size
#define RUNSIZE 32
//#define HEAPSIZE 2048
#define HEAPSIZE 6000

// Don't pack but align. This is necessary for the crappy msp430-gcc compiler.
// #define PACK_STRUCTS
#define ALIGN_16

//use 32-bit time as int64_t support is broken for the msp430x compiler
#include<sys/types.h>
typedef int32_t dj_time_t;

#include<stdarg.h>
void g301_serialVPrint(char * format, va_list arg);
void g301_serialPrintf(char * format, ...);

/* Please see common/debug.h */
//#define DARJEELING_DEBUG
//#define DARJEELING_DEBUG_TRACE
//#define DARJEELING_DEBUG_CHECK_HEAP_SANITY
//#define DARJEELING_DEBUG_PERFILE
//#define ASSEMBLY_DEBUGGING 1
//#define ASSEMBLY_DEBUG nesc_printf
#define DARJEELING_PRINTF g301_serialPrintf
#define DARJEELING_PGMSPACE_MACRO(x) (x)

#endif
