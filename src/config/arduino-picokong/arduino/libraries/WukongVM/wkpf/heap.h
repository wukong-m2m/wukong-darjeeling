/*
 * heap.h
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
 
#ifndef __heap_h
#define __heap_h

#include "config.h"
#include "types.h"
#include "hooks.h"

// To let other libraries hook into the garbage collector.
extern dj_hook *dj_mem_markRootSetHook;
extern dj_hook *dj_mem_markObjectHook;
extern dj_hook *dj_mem_updateReferenceHook;
extern dj_hook *dj_mem_postGCHook;

#define SAFE_POINTER_POOL_SIZE 4

/**
 * Heap chunk types. Darjeeling keeps a heap where Java objects and non-Java objects are mixed on the same heap.
 * This enumeration indicates the types of built-in objects.
 */
// CHUNKID_FREE acts as a marker. Everything above CHUNKID_FREE is a built-in object and is never garbage collected
// Please don't change the order of things here
enum chunk_id
{
	CHUNKID_FREE=0,
	CHUNKID_INVALID=1,

	CHUNKID_MONITOR_BLOCK=2,
	CHUNKID_VM=3,
	CHUNKID_FRAME=4,
	CHUNKID_THREAD=5,

	CHUNKID_MANAGED_START=6,

	CHUNKID_INFUSION=6,
	CHUNKID_REFARRAY=7,
	CHUNKID_INTARRAY=8,

	CHUNKID_WUCLASS=9,
	CHUNKID_WUOBJECT=10,

	CHUNKID_JAVA_START=11

};

typedef struct _heap_chunk heap_chunk;

/**
 *
 */
enum
{
	TCM_WHITE = 0,
	TCM_GRAY = 1,
	TCM_BLACK = 2,
};

// TODO the size:14 bit field notation is a GCC extension - refactor for compatibility with other compilers
struct _heap_chunk
{
	uint16_t color:2;
	uint16_t size:14;
	uint16_t shift;
	uint8_t id;
#ifdef ALIGN_16
	uint8_t PADDING;
#endif
}
#ifdef PACK_STRUCTS
__attribute__ ((__packed__))
#endif
;

void dj_mem_init(void *mem_pointer, uint16_t mem_size);
void * dj_mem_alloc(uint16_t size, runtime_id_t id);
uint16_t dj_mem_getFree();
uint16_t dj_mem_getSize();

void dj_mem_free(void *ptr);

void dj_mem_addSafePointer(void ** ptr);
void dj_mem_removeSafePointer(void ** ptr);

int dj_mem_countChunks(runtime_id_t id);
heap_chunk * dj_mem_getFirstChunk();
heap_chunk * dj_mem_getNextChunk(heap_chunk *currentChunk);

void dj_mem_gc();

void * dj_mem_getPointer();

void dj_mem_shiftRuntimeIDs(runtime_id_t start, uint16_t range);

#ifdef DARJEELING_DEBUG
void dj_mem_dump();
#endif

#ifdef DARJEELING_DEBUG_CHECK_HEAP_SANITY
void dj_check_sanity();
#endif

static inline void * dj_mem_getData(heap_chunk * chunk)
{
	return (void*)((size_t)(chunk) + sizeof(heap_chunk));
}

static inline runtime_id_t dj_mem_getChunkId(void *ptr)
{
	return ((heap_chunk*)((size_t)ptr-sizeof(heap_chunk)))->id;
}

static inline void dj_mem_setChunkId(void *ptr, uint8_t id)
{
	((heap_chunk*)((size_t)ptr-sizeof(heap_chunk)))->id = id;
}

static inline uint16_t dj_mem_getChunkSize(void *ptr)
{
	return ((heap_chunk*)((size_t)ptr-sizeof(heap_chunk)))->size;
}

static inline uint16_t dj_mem_getChunkShift(void *ptr)
{
	return ((heap_chunk*)((size_t)ptr-sizeof(heap_chunk)))->shift;
}

static inline void * dj_mem_getUpdatedPointer(void * ptr)
{
	return (ptr==0)? 0 : (void*)((size_t)ptr - dj_mem_getChunkShift(ptr));
}
#ifndef ARDUINO_ARCH_WUDEVICE
static inline ref_t dj_mem_getUpdatedReference(ref_t ref)
{
	return (ref==0)?0:(ref - dj_mem_getChunkShift(REF_TO_VOIDP(ref)));
}
#endif

static inline void dj_mem_setChunkColor(void *ptr, int color)
{
	((heap_chunk*)((size_t)ptr - sizeof(heap_chunk)))->color = color;
}

static inline int dj_mem_getChunkColor(void *ptr)
{
	return ((heap_chunk*)((size_t)ptr - sizeof(heap_chunk)))->color;
}

static inline void dj_mem_setRefGrayIfWhite(ref_t ref)
{
	if (ref==nullref) return;
	heap_chunk * chunk = ((heap_chunk*)((size_t)REF_TO_VOIDP(ref) - sizeof(heap_chunk)));
	if (chunk->color==TCM_WHITE) chunk->color=TCM_GRAY;
}

static inline void dj_mem_setPointerGrayIfWhite(void * ptr)
{
	if (ptr == NULL) return;
	heap_chunk * chunk = ((heap_chunk*)((size_t)ptr - sizeof(heap_chunk)));
	if (chunk->color==TCM_WHITE) chunk->color=TCM_GRAY;
}

static inline void dj_mem_setRefColor(ref_t ref, int color)
{
	if (ref==nullref) return;
	dj_mem_setChunkColor(REF_TO_VOIDP(ref), color);
}

#endif
