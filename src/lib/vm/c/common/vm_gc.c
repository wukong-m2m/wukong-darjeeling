#include "core.h"
#include "heap.h"
#include "debug.h"
#include "panic.h"
#include "hooks.h"
#include "types.h"
#include "heap.h"

#include "vm.h"
#include "vmthread.h"
#include "infusion.h"
#include "array.h"
#include "execution.h"
#include "jlib_base.h"


static dj_object *panicExceptionObject = NULL;

static inline void vm_mem_updateManagedReference(dj_vm * vm, heap_chunk *chunk)
{
	int i;
	dj_di_pointer classDef;
	ref_t *refs;

	if (chunk->id>=CHUNKID_JAVA_START)
	{
		// object
                classDef = dj_vm_getRuntimeClassDefinition(chunk->id);
                //refs = dj_object_getReferences((dj_object*)chunk);
		refs = dj_object_getReferences(dj_mem_getData(chunk));
		for (i=0; i<dj_di_classDefinition_getNrRefs(classDef); i++)
			refs[i] = dj_mem_getUpdatedReference(refs[i]);
	}

	if (chunk->id==CHUNKID_REFARRAY)
	{
		dj_ref_array_updatePointers((dj_ref_array*)dj_mem_getData(chunk));
	}

}

static inline void vm_mem_updateSystemReference(heap_chunk *chunk)
{
	switch (chunk->id)
	{

		case CHUNKID_INFUSION:
			dj_infusion_updatePointers((dj_infusion*)dj_mem_getData(chunk));
			break;

		case CHUNKID_VM:
			dj_vm_mem_updatePointers((dj_vm*)dj_mem_getData(chunk));
			break;

		case CHUNKID_THREAD:
			dj_thread_updatePointers((dj_thread*)dj_mem_getData(chunk));
			break;

		case CHUNKID_MONITOR_BLOCK:
			dj_monitor_block_updatePointers((dj_monitor_block*)dj_mem_getData(chunk));
			break;

		// this is now handled from dj_thread_updatePointers
		// case CHUNKID_FRAME:
		// 	dj_frame_updatePointers((dj_frame*)dj_mem_getData(chunk));
		// 	break;

		default:
			break;

	}
}

void vm_mem_setPanicExceptionObject(dj_object *obj)
{
	panicExceptionObject = obj;
}

dj_object * vm_mem_getPanicExceptionObject()
{
	return panicExceptionObject;
}


// vm_mem_pre/postGC: This code used to be in heap.c, at the beginning and end of dj_mem_gc()
// these functions will be called from the beginning of vm_mem_markRootSet, and the end of 
// vm_mem_updatePointers, which should have the same result.
// I'm putting them in two separate functions to keep the structure clear since this code should
// run before and after all other vm related GC code.
void vm_mem_preGC() { // This is called from vm_mem_markRootSet, which is early enough
	dj_vm *vm = dj_exec_getVM();
	if (vm == NULL)
	{
		DEBUG_LOG(DBG_DARJEELING, "GC: VM not yet init\n");
		dj_panic(DJ_PANIC_OUT_OF_MEMORY);
	}

	// Force the execution engine to store the nr_int_stack and nr_ref_stack in the current frame struct
	// we need this for the root set marking phase
	dj_thread * thread = dj_exec_getCurrentThread();
	if (thread && thread->frameStack)
		dj_exec_deactivateThread(thread);
}
void vm_mem_postGC(void *data) { // This is called by the GC's dj_mem_postGCHook
	dj_thread * thread = dj_exec_getCurrentThread();
	if (thread && thread->frameStack)
		dj_exec_activate_thread(thread);
}

////// Begin hooks from GC
/**
 * Marks the root set. The root set can be marked directly in one pass since reference and non-reference types
 * are separated.
 * @param vm the virtual machine context
 */
void vm_mem_markRootSet(void *data)
{
	dj_vm *vm = dj_exec_getVM();
	dj_thread * thread;
	dj_infusion * infusion;
	dj_monitor_block * monitorBlock;

	vm_mem_preGC();

	DEBUG_LOG(DBG_DARJEELING, "\t\tmark threads\n");

	// Mark threads
	thread = vm->threads;
	while (thread!=NULL)
	{
		dj_thread_markRootSet(thread);
		thread = thread->next;
	}

	DEBUG_LOG(DBG_DARJEELING, "\t\tmark infusions\n");

    // Mark infusions
	infusion = vm->infusions;
	while (infusion!=NULL)
	{
		dj_infusion_markRootSet(infusion);
		infusion = infusion->next;
	}

	DEBUG_LOG(DBG_DARJEELING, "\t\tmark monitors\n");

	// Mark monitor blocks
	monitorBlock = vm->monitors;
	while (monitorBlock!=NULL)
	{
		dj_monitor_markRootSet(monitorBlock);
		monitorBlock = monitorBlock->next;
	}

	// mark the panic exception object
	if (panicExceptionObject!=nullref) {
		DBG_PRINT_GC(0x10000101);
		dj_mem_setRefGrayIfWhite(VOIDP_TO_REF(panicExceptionObject));
	}
}

/**
 * Implements the marking phase using stop-the-world tri-color marking
 */
void vm_mem_markObject(void *data)
{
	heap_chunk *chunk = (heap_chunk *)data;
	dj_di_pointer classDef;
	dj_ref_array *refArray;
	void * object;
	int i;
	ref_t *refs;

	// get the chunk that contains our object
	object = dj_mem_getData(chunk);

	// if it's a Java object, mark its children
	if (chunk->id>=CHUNKID_JAVA_START)
	{
		classDef = dj_vm_getRuntimeClassDefinition(chunk->id);
		refs = dj_object_getReferences(object);
		for (i=0; i<dj_di_classDefinition_getNrRefs(classDef); i++) {
			DBG_PRINT_GC(0x10000102);
			dj_mem_setRefGrayIfWhite(refs[i]);
		}
	}

	// if it's a reference array, mark the members
	if (chunk->id==CHUNKID_REFARRAY)
	{
		refArray = (dj_ref_array*)object;
		for (i=0; i<refArray->array.length; i++) {
			DBG_PRINT_GC(0x10000103);
			dj_mem_setRefGrayIfWhite(refArray->refs[i]);
		}
	}

	// mark the object as 'black'
	chunk->color = TCM_BLACK;
}

void vm_mem_updatePointers(void *data) {
	heap_chunk *chunk;

	// Update the managed pointers in each of the chunks.
	// The pointers are updated in two passes because to update the heap objects we
	// need the vm and infusions in tact.
	dj_vm *vm = dj_exec_getVM();
	chunk = dj_mem_getFirstChunk();
	while (chunk)
	{
		vm_mem_updateManagedReference(vm, chunk);
		chunk = dj_mem_getNextChunk(chunk);
	}

	// When the Java objects have been updated, we can safely update the system
	// objects.
	chunk = dj_mem_getFirstChunk();
	while (chunk)
	{
		vm_mem_updateSystemReference(chunk);
		chunk = dj_mem_getNextChunk(chunk);
	}

	// update global pointers in the execution engine
	dj_exec_updatePointers();

	// update the pointer to the panic exception object, if any
	if (panicExceptionObject!=nullref)
		panicExceptionObject = dj_mem_getUpdatedPointer(panicExceptionObject);
}
////// End hooks from GC

#ifdef DARJEELING_DEBUG_MEM_TRACE
void vm_mem_dumpMemUsage()
{
	heap_chunk *finger;
	dj_thread *thread;
	dj_frame *frame;
	dj_vm *vm = dj_exec_getVM();
	int id, total, grandTotal;

	if (vm==NULL)
		return;

	printf("TRACE %d", nrTrace);

	grandTotal = 0;

	// output the first 8 threads
	for (id=0; id<8; id++)
	{
		thread = dj_vm_getThreadById(vm, id);
		if (thread!=NULL)
		{
			total = dj_mem_getChunkSize(thread) - (4 * 2);

			frame = thread->frameStack;
			while (frame!=NULL)
			{
				total += dj_mem_getChunkSize(frame) - (2 * 2);
				frame = frame->parent;
			}

		} else
			total = 0;

		grandTotal += total;

		printf(", %d", total);
	}

	thread = dj_exec_getCurrentThread();
	if (thread==NULL)
		printf(", %d", -1);
	else
		printf(", %d", dj_exec_getCurrentThread()->id);

	printf(", %d", grandTotal);

	printf("\n");
	nrTrace++;
}
#endif // #ifdef DARJEELING_DEBUG_MEM_TRACE

