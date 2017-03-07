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
#include <stdint.h>

// define heap size
#define RUNSIZE 32
#define HEAPSIZE 2048

// define wether to pack structs (this is fine on all AVR targets)
#define PACK_STRUCTS
	
//Use 64-bit values to store time
typedef int64_t dj_time_t;

/* Please see common/debug.h */
// Note that the TNODE platform only has 4k of memory. Enabling DEBUG 
// causes a lot of that memory to be stuffed with debug strings, so you'll
// have to free up memory by reducing HEAPSIZE to something like 512 bytes
// if you want to see full debug traces on this platform.
// #define DARJEELING_DEBUG
// #define DARJEELING_DEBUG_TRACE
// #define DARJEELING_DEBUG_CHECK_HEAP_SANITY
// #define DARJEELING_DEBUG_PERFILE
#define DARJEELING_PRINTF(...)
#define DARJEELING_PGMSPACE_MACRO

#endif
