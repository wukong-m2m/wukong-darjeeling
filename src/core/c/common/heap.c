/*
 * heap.c
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
 
/**
 * \defgroup heap Heap management
 * \ingroup heap
 * @{
 * \page heap_description
 *
 * Darjeeling manages its own heap using garbage collection. Java objects and internal objects (threads, stack frames,
 * infusions etc) are mixed. Each heap element (chunk) carries a byte that indicates its type. Some type numbers are
 * reserved for internal objects or arrays, the other numbers are used for Class ID's.
 *
 * The Heap manager implements a simple mark & compact garbage collector. Blocks are allocated using pointer bumping
 * for quick allocation.
 *
 * The marking algorithm is a stop-the-world tri-color marker. The root set is first marked directly and set to gray
 * or black. The heap is then sweeped pulling each gray block into the black set by following its references and
 * marking them gray. The heap is sweeped in consecutive passes until there are no more gray elements on the heap.
 *
 * Because Darjeeling separates reference and integer values in allocated Objects and on the stack, the root set can be
 * marked directly by following these reference values. This means that there are no false positives in the root set
 * and that marking the root set is done efficiently.
 *
 * Eventually the code from this garbage collector should be merged with the code from the old mark & sweep collector
 * so that allocation is done using a free list and compaction is only triggered when absolutely neccesairy.
 *
 * Because calls to dj_mem_alloc() can trigger garbage compaction, pointers (in the C code) may be made invalid as the
 * element they were pointing to may have been moved. A C pointer can be added to the 'safe pointer pool' which will cause
 * the object the pointer is pointing to to be marked as live, and the pointer to be updated in case of GC. The size of
 * the pool is limited, so it's best to add pointers only temporarily (i.e. during method calls where multiple allocations
 * are needed).
 *
 * For example:
 *
 * <code>
 *   // pop an object off the operand stack<br>
 *   dj_object * object = REF_TO_VOIDP(dj_exec_stackPopRef());<br>
 *   <br>
 *   // make sure the object pointer is updated on garbage compaction<br>
 *   dj_mem_addSafePointer((void**)&object);
 *   <br>
 *   // do some memory allocation<br>
 *   void * something = dj_mem_alloc(...);<br>
 *   <br>
 *   // remove the pointer from the safe pool when done
 *   dj_mem_removeSafePointer((void**)&object);
 *   <br>
 * </code>
 *
 */

#include <string.h>

#include "config.h"
#include "core.h"
#include "heap.h"
#include "debug.h"
#include "panic.h"
#include "hooks.h"

static char *heap_base;
static uint16_t heap_size;

static void ** safePointerPool[SAFE_POINTER_POOL_SIZE];

static void *left_pointer, *right_pointer;

#ifdef DARJEELING_DEBUG_MEM_TRACE
static int nrTrace = 0;
#endif

// To let other libraries hook into the garbage collector.
dj_hook *dj_mem_markRootSetHook = NULL;
dj_hook *dj_mem_markObjectHook = NULL;
dj_hook *dj_mem_updateReferenceHook = NULL;
dj_hook *dj_mem_postGCHook = NULL;

/**
 * Initialises the memory manager. A call to this function may trigger garbage collection.
 * @param mem_pointer pointer to where Darjeeling can manage its heap
 * @param mem_size size of the heap
 */
void dj_mem_init(void *mem_pointer, uint16_t mem_size)
{
	uint16_t i;

    // initialize our private pointer to the heap memory
    heap_base =  mem_pointer;

    // the rest of the memory is for heap chunks
    heap_size = mem_size;

    for (i=0; i<SAFE_POINTER_POOL_SIZE; i++) safePointerPool[i] = NULL;

    left_pointer = heap_base;
    right_pointer = heap_base + heap_size;

}

void * dj_mem_checked_alloc(uint16_t size, uint16_t id) {
	void *p = dj_mem_alloc(size, id);
	if (p == NULL) {
		avroraPrintHex32(0xDEADDEAD);
		avroraPrintInt16(dj_mem_getFree());
		avroraPrintInt16(size);
    	dj_panic(DJ_PANIC_OUT_OF_MEMORY);
	}
	return p;
}


/**
 * Allocates <emph>size</emph> bytes.
 * @param size size in bytes
 * @param id chunk ID.
 */
void * dj_mem_alloc(uint16_t size, uint16_t id)
{
	heap_chunk *ret;

	// if ALIGN_16 is defined (for MSP430 platform) make sure the size of the
	// new chunk is a multiple of 2
#ifdef ALIGN_16
	if (size&1) size++;
#endif

	// we need to accommodate for a chunk header
	size += sizeof(heap_chunk);

	if (right_pointer-left_pointer<size)
	{
		// not enough memory
        DEBUG_LOG(DBG_DARJEELING, "dj_mem_alloc: triggering a collection\n");
        avroraPrintInt16(size);
		dj_mem_gc();

		if (right_pointer-left_pointer<size)
		{
			// still not enough memory, return null
	        DEBUG_LOG(DBG_DARJEELING, "dj_mem_alloc: NULL!\n");
	        return NULL;
		}
	}

	ret = (heap_chunk*)left_pointer;
	ret->size = size;
	ret->id = id;

	left_pointer += size;

	return (void*)((size_t)ret + sizeof(heap_chunk));
}

/**
 * Frees memory on the heap.
 * @ptr pointer to memory
 */
void dj_mem_free(void * ptr)
{
	heap_chunk *chunk = (heap_chunk*)(ptr - sizeof(heap_chunk));

	if (((void*)chunk + chunk->size)==left_pointer)
		left_pointer = (void*)chunk;
	else
		chunk->id = CHUNKID_FREE;

}

/**
 * @return the total number of bytes on the heap.
 */
uint16_t dj_mem_getSize()
{
	return heap_size;
}

/**
 * @return the number of bytes left on the heap.
 */
uint16_t dj_mem_getFree()
{
	return right_pointer - left_pointer;
}

/**
 * Pushes a pointer on the safe pointer stack.
 * @param void** the pointer to keep updated during compaction.
 */
void dj_mem_addSafePointer(void ** ptr)
{
	uint16_t i;

	for (i=0; i<SAFE_POINTER_POOL_SIZE; i++)
		if (safePointerPool[i]==NULL)
		{
			safePointerPool[i] = ptr;
			return;
		}

	dj_panic(DJ_PANIC_SAFE_POINTER_OVERFLOW);
}

uint16_t dj_mem_countSafePointers()
{
	uint16_t ret = 0, i;

	for (i=0; i<SAFE_POINTER_POOL_SIZE; i++)
		if (safePointerPool[i]!=NULL) ret ++;

	return ret;
}

/**
 * Pops a reference from the compaction update stack.
 * @return updated reference
 */
void dj_mem_removeSafePointer(void ** ptr)
{
	uint16_t i;

	for (i=0; i<SAFE_POINTER_POOL_SIZE; i++)
		if (safePointerPool[i]==ptr)
			safePointerPool[i] = NULL;

}

/**
 * Shifts the runtime IDs of objects on the heap to correct ranges when infusions are unloaded.
 * Any id within [start, start+range> is marked invalid (as the infusion that holds the Class
 * definition for that object is unloaded), and IDs in [start+range, ->> are mapped to [start,->>
 */
void dj_mem_shiftRuntimeIDs(runtime_id_t start, uint16_t range)
{

	heap_chunk *chunk;
	void * loc = heap_base;
	runtime_id_t id;

	// initialise all chunk headers to WHITE
	while (loc<left_pointer)
	{
		chunk = (heap_chunk*)loc;
		id = chunk->id;

		// if the ID is inside [start, start+range>, it belongs to the unloaded
		// infusion, so mark the object 'invalid'
		if (id>start&&id<(start+range))
			chunk->id = CHUNKID_INVALID;

		// if the ID is in [start+range, ->>, it belongs to some infusion with IDs
		// that are higher than the one in the unloaded infusion, so they should be
		// shifted back
		if (id>start+range)
			chunk->id-=range;

		loc += chunk->size;
	}


}

/**
 * We'll add hooks to object finalisers here later
 */
static inline void dj_finalise(heap_chunk * chunk)
{
	switch (chunk->id)
	{

	default:
		chunk->id = CHUNKID_FREE;
		// nothing

	}
}

/**
 * Implements the marking phase using stop-the-world tri-color marking
 */
static inline void dj_mem_mark()
{
	heap_chunk *chunk;
	void * loc = heap_base;
	uint8_t i;

	// Initialise chunk colors to white for managed objects (Java objects, infusions), and
	// black for built-in objects (frames, threads)
	while (loc<left_pointer)
	{
		chunk = (heap_chunk*)loc;
		chunk->color = chunk->id<CHUNKID_MANAGED_START?TCM_BLACK:TCM_WHITE;
		loc += chunk->size;
	}

	DEBUG_LOG(DBG_DARJEELING, "\tmark root set\n");

	// mark the root set (set all elements in the root set to 'gray')
	dj_hook_call(dj_mem_markRootSetHook, NULL);

	DEBUG_LOG(DBG_DARJEELING, "\tmark reference stack\n");

	// mark the safe pointer pool
	for (i=0; i<SAFE_POINTER_POOL_SIZE; i++)
		if (safePointerPool[i]!=NULL)
			dj_mem_setPointerGrayIfWhite(*(safePointerPool[i]));

	// iterate over the chunks, make every gray chunk black
	int nrGray;
	do
	{
		DEBUG_LOG(DBG_DARJEELING, "\titeration\n");

		// move over the chunks and visit all the gray objects
		nrGray=0;
		loc = heap_base;
		while (loc<left_pointer)
		{
			chunk = (heap_chunk*)loc;

			if (chunk->color==TCM_GRAY)
			{
				dj_hook_call(dj_mem_markObjectHook, (void *)chunk);
				nrGray++;
			}

			loc += chunk->size;
		}

	} while (nrGray>0);

	// Call finalise on each of the chunks
	loc = heap_base;
	while (loc<left_pointer)
	{
		chunk = (heap_chunk*)loc;
		if (chunk->color==TCM_WHITE)
			dj_finalise(chunk);
		loc += chunk->size;
	}
}

heap_chunk * dj_mem_getFirstChunk() {
	return (heap_chunk *)heap_base;
}

heap_chunk * dj_mem_getNextChunk(heap_chunk *currentChunk) {
	void *loc = (void *)currentChunk;
	if (loc+currentChunk->size < left_pointer)
		return loc+currentChunk->size;
	else
		return NULL;
}

void dj_mem_compact()
{
	int i;

	heap_chunk *chunk;
	uint32_t shift = 0;
	uint32_t chunkSize;
	void * loc;

	// calculate how many bytes each non-free chunk will be shifted by compaction
	// and store this number in the chunk header
	loc = heap_base;
	while (loc<left_pointer)
	{
		chunk = (heap_chunk*)loc;

		if (chunk->id==CHUNKID_FREE) shift += chunk->size;

		chunk->shift = shift;
		loc += chunk->size;
	}

	// update pointers in the safe pointer pool
	for (i=0; i<SAFE_POINTER_POOL_SIZE; i++)
		if (safePointerPool[i]!=NULL)
			*(safePointerPool[i]) = dj_mem_getUpdatedPointer(*(safePointerPool[i]));

	// Let the libraries update the references held in the chunks
	dj_hook_call(dj_mem_updateReferenceHook, NULL);

	// physically move the chunks to their new positions
	loc = heap_base;
	while (loc<left_pointer)
	{
		chunk = (heap_chunk*)loc;
		chunkSize = chunk->size;

		if (chunk->id!=CHUNKID_FREE)
			memmove(loc-chunk->shift, loc, chunkSize);

		loc += chunkSize;
	}

	left_pointer-=shift;
}

void dj_mem_gc()
{
	avroraPrintStr("gc");
	avroraPrintFreeHeapMemory();
    avroraStartGarbageCollectionTimer();
	DEBUG_LOG(DBG_DARJEELING | DBG_DARJEELING_GC, "GC start\n");

	dj_mem_mark();
	dj_mem_compact();
	dj_hook_call(dj_mem_postGCHook, NULL);

	DEBUG_LOG(DBG_DARJEELING | DBG_DARJEELING_GC, "GC done\n");
    avroraStopGarbageCollectionTimer();
	avroraPrintFreeHeapMemory();
	avroraPrintStr("gc done");
}

bool dj_mem_isHeapPointer(void *ptr) {
	return (void *)heap_base <= ptr && ptr <= (void *)right_pointer;
}

//void dj_mem_thread_dump()
//{
//
//	dj_vm *vm = dj_exec_getVM();
//	if (vm==NULL)
//		return;
//
//	int i, threadTotal, grandTotal;
//	int nrThreads = dj_vm_countThreads(vm);
//	dj_thread *thread;
//	dj_frame *frame;
//
//	time++;
//	grandTotal = 0;
//
//	int threads[16];
//	for (i=0; i<16; i++)
//		threads[i] = 0;
//
//	if (nrThreads>0)
//	{
//		for (i=0; i<nrThreads; i++)
//		{
//			thread = dj_vm_getThread(vm, i);
//			threadTotal = dj_mem_get_chunk_size(thread) - 10;
//
//			for (frame=thread->frameStack; frame!=NULL; frame=frame->parent)
//				threadTotal += dj_mem_get_chunk_size(frame) - 4;
//
//			threads[thread->id] = threadTotal;
//			grandTotal += threadTotal;
//		}
//		printf("%d %d %d %d %d\n",
//				time,
//				threads[1],
//				threads[2],
//				threads[3],
//				grandTotal);
//	}
//
//}

void dj_mem_dump()
{
	heap_chunk *finger;

	// iterate over all blocks
	finger = (heap_chunk*)heap_base;

	int total = 0;

	while ( (void *)finger < left_pointer )
    {
    	avroraPrintHex32(0xbbbbbbbb);
        avroraPrintPtr(finger);
        avroraPrintUInt16(finger->id);
        avroraPrintUInt16(finger->size);
        avroraPrintInt16(finger->shift);
		total += finger->size;
		if (finger->size==0)
			break;
		finger = (heap_chunk*)((char*)finger + finger->size);
	}

	avroraPrintHex32(0xcccccccc);
	avroraPrintInt16(total);
}

#ifdef DARJEELING_DEBUG
void dj_mem_dump()
{
	heap_chunk *finger;

	// iterate over all blocks
	finger = (heap_chunk*)heap_base;

	int total = 0;

	while ( (void *)finger < left_pointer )
    {
        avroraPrintPtr(finger);
        // char pretty_printing_buffer[5];
        // char *chunk_type_pretty_print;
        switch(finger->id)
        {
            case CHUNKID_REFARRAY:
                avroraPrintStr("REFA");
                break;
            case CHUNKID_INTARRAY:
                avroraPrintStr("INTA");
                break;
            case CHUNKID_FREE:
                avroraPrintStr("FREE");
                break;
            case CHUNKID_INVALID:
                avroraPrintStr("INV");
                break;
            case CHUNKID_MONITOR_BLOCK:
                avroraPrintStr("MTBK");
                break;
            case CHUNKID_VM:
                avroraPrintStr("  VM");
                break;
            case CHUNKID_INFUSION:
                avroraPrintStr("IFSN");
                break;
            case CHUNKID_FRAME:
                avroraPrintStr("STKF");
                break;
             case CHUNKID_THREAD:
                avroraPrintStr("THRD");
                break;
            default:
                avroraPrintStr("????");
                break;

        }

		// // printf("%c[32mASSERT[%3d] PASSED%c[0m\n", 0x1b, (int)id, 0x1b);
  //       uint8_t color = 31 + finger->color;
		// DEBUG_LOG(1, "%c[%dm[%p %04d %s]%c[0m \n", 0x1b, color, finger, finger->size, chunk_type_pretty_print, 0x1b);
		total += finger->size;
		if (finger->size==0)
			break;
		finger = (heap_chunk*)((char*)finger + finger->size);
	}

	avroraPrintStr("totaal");
	avroraPrintInt16(total);
	// DEBUG_LOG(1, "\n");
	// DEBUG_LOG(1, "total %d\n", total);

}
#endif // ifdef DARJEELING_DEBUG


