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
#include <sys/types.h>

#define MEMSIZE 4096
#define RUNSIZE 128

#define PACK_STRUCTS
// #define ALIGN_16
//Use 64-bit values to store time
typedef int64_t dj_time_t;

/* Please see common/debug.h */

// #define DARJEELING_DEBUG
// #define DARJEELING_DEBUG_TRACE
// #define DARJEELING_DEBUG_CHECK_HEAP_SANITY
// #define DARJEELING_DEBUG_PERFILE
#define DARJEELING_PRINTF printf
#define DARJEELING_PGMSPACE_MACRO

#endif
