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

// define heap size
#define RUNSIZE 32
#define HEAPSIZE 4096

// define wether to pack structs (this is fine on all AVR targets)
#define PACK_STRUCTS
//Use 64-bit values to store time
typedef int64_t dj_time_t;

/* Please see common/debug.h */

//#define DARJEELING_DEBUG
//#define DARJEELING_DEBUG_TRACE
//#define DARJEELING_DEBUG_CHECK_HEAP_SANITY
//#define DARJEELING_DEBUG_PERFILE
#ifndef __TEXT_BUF__
#define __TEXT_BUF__
#include <stdio.h>
#include <stdlib.h>
char *textBuf;
#endif
//this define is used to let vm be stored as a global variable in DarjeelingC.nc
#define IS_TOSSIM
//this define is used to include excpetion text messages
#define IS_SIMULATOR
//#define DARJEELING_PRINTF(...)
#define DARJEELING_PRINTF(...) textBuf = malloc(100);\
							if (textBuf != NULL) { snprintf(textBuf, 100, __VA_ARGS__);\
							tossim_debug(textBuf);\
							free(textBuf);}


#define DARJEELING_PGMSPACE_MACRO



#define ASSEMBLY_DEBUGGING 0
#if ASSEMBLY_DEBUGGING
#ifndef __TEXT_BUF__
#define __TEXT_BUF__
#include <stdio.h>
char *textBuf;
#endif
#define ASSEMBLY_DEBUG(...) textBuf = dj_mem_alloc(65, CHUNKID_REFARRAY);\
							if (textBuf != NULL) { snprintf(textBuf, 65, __VA_ARGS__);\
							tossim_debug(textBuf);\
							dj_mem_free(textBuf);}
#else
#define ASSEMBLY_DEBUG(...)
#endif
#endif
