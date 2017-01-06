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
#include<stdio.h>
#include <sys/types.h>

// define heap size
#define RUNSIZE 128
#define HEAPSIZE 8192

// define wether to pack structs (this is fine on all AVR targets)
// don't pack structs on MSP430 targets
// #define PACK_STRUCTS
#define ALIGN_16
//Use 64-bit values to store time
typedef int64_t dj_time_t;

// #define HAS_USART

/* Please see common/debug.h */
//#define DARJEELING_DEBUG
//#define DARJEELING_DEBUG_TRACE
//#define DARJEELING_DEBUG_CHECK_HEAP_SANITY
//#define DARJEELING_DEBUG_PERFILE
#define DARJEELING_PRINTF(...) printf(__VA_ARGS__)
#define DARJEELING_PGMSPACE_MACRO PSTR

#define IS_COOJA

#define ASSEMBLY_DEBUGGING 0
#if ASSEMBLY_DEBUGGING
#include "stdio.h"
extern FILE *logFile;
#define ASSEMBLY_DEBUG(...) fprintf(logFile, __VA_ARGS__)
#else
#define ASSEMBLY_DEBUG(...)
#endif

#endif
