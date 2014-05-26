/*
 * vm.c
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
 
#include "heap.h"
#include "global_id.h"
#include "execution.h"
#include "util.h"
#include "infusion.h"
#include "debug.h"
#include "panic.h"
#include "djtimer.h"
#include "vm.h"
#include "djarchive.h"
#include "core.h"
#include "vm_gc.h"
#include "jlib_base.h"

/**
 *
 *
 * @author Niels Brouwers
 */


// TMPRTC
extern void rtc_compile_lib(dj_infusion *infusion);


/**
 * Main function that creates the vm and runs until either all threads stop or runlevel is set to RUNLEVEL_REBOOT
 * This code was in the main() of each config before, but has been extracted to make rebooting easier.
 */
void dj_vm_main(dj_di_pointer di_lib_infusions_archive_data,
 				dj_di_pointer di_app_infusion_archive_data,
 				dj_named_native_handler handlers[],
 				uint8_t handlers_length) {
	dj_vm *vm;
	dj_object * obj;

	// create a new VM
	vm = dj_vm_create();

	// store the application archive
	vm->di_app_infusion_archive_data = di_app_infusion_archive_data;

	// tell the execution engine to use the newly created VM instance
	dj_exec_setVM(vm);
	// set run level before loading libraries since they need to execute initialisation code
	dj_exec_setRunlevel(RUNLEVEL_RUNNING);

	dj_vm_loadInfusionArchive(vm, di_lib_infusions_archive_data, handlers, handlers_length);
	dj_di_pointer di_app_infusion_data = dj_archive_get_file(di_app_infusion_archive_data, 0);
	dj_vm_loadInfusion(vm, di_app_infusion_data, NULL, 0);

	// TMPRTC
	{
		dj_infusion *finger = vm->infusions;

		while (finger!=NULL)
		{
			dj_di_pointer name = dj_di_header_getInfusionName(finger->header);
			if (dj_di_getU8(name)=='r' && dj_di_getU8(name+1)=='t' && dj_di_getU8(name+2)=='c') {
				rtc_compile_lib(finger);
				break;
			}
			finger = finger->next;
		}
		if (finger == NULL) {
			DEBUG_LOG(true, "rtc niet gevonden.\n");
			dj_panic(DJ_PANIC_MALFORMED_INFUSION);
		}
	}
	// ENDTMPRTC

	// pre-allocate an OutOfMemoryError object
	obj = dj_vm_createSysLibObject(vm, BASE_CDEF_java_lang_OutOfMemoryError);
	vm_mem_setPanicExceptionObject(obj);

	DEBUG_LOG(true, "Darjeeling is go!\n\r");

	// start the main execution loop
	while (dj_vm_countLiveThreads(vm)>0)
	{
		dj_vm_schedule(vm);
		if (vm->currentThread!=NULL)
			if (vm->currentThread->status==THREADSTATUS_RUNNING)
				dj_exec_run(RUNSIZE);
	}
	DEBUG_LOG(true, "All threads terminated.\n\r");
}


/**
 * Constructs a new virtual machine context.
 * @return a newly constructed virtual machine instance or NULL if fail (out of memory)
 */
dj_vm * dj_vm_create()
{
	dj_vm *ret = (dj_vm*)dj_mem_alloc(sizeof(dj_vm), CHUNKID_VM);

    if(ret == NULL) return NULL;

	// initialise linked lists to 0 elements
	ret->infusions = NULL;
	ret->threads = NULL;

	// no threads running
	ret->currentThread = NULL;
	ret->threadNr = 0;

	// no monitors
	ret->monitors = NULL;
	ret->numMonitors = 0;

	// no system infusion loaded
	ret->systemInfusion = NULL;

	return ret;
}

/**
 * Destroys a virtual machine instance
 */
void dj_vm_destroy(dj_vm *vm)
{
	dj_mem_free(vm);
}

/**
 * Adds an infusion to a virtual machine.
 * @param vm the virtual machine to add the infusion to
 * @param infusion the infusion to add
 */
void dj_vm_addInfusion(dj_vm *vm, dj_infusion *infusion)
{
	dj_infusion *tail = vm->infusions;
	if (tail!=NULL)
		while (tail->next!=NULL)
			tail = tail->next;

	if (tail==NULL)
	{
		// list is empty, add as first element
		vm->infusions = infusion;
		infusion->class_base = CHUNKID_JAVA_START;
	}
	else
	{
		// add to the end of the list
		tail->next = infusion;
		infusion->class_base = tail->class_base + dj_di_parentElement_getListSize(tail->classList);
	}

	// the new infusion is the last element,
	// so its next should be NULL
	infusion->next = NULL;


}

/**
 * Counts the number of loaded infusions in a virtual  machine.
 * @param vm the virtual machine to count the infusions of
 * @return the number of loaded infusions
 */
int dj_vm_countInfusions(dj_vm *vm)
{
	int ret = 0;
	dj_infusion *tail = vm->infusions;

	while (tail!=NULL)
	{
		tail = tail->next;
		ret++;
	}

	return ret;

}

uint8_t dj_vm_getInfusionId(dj_vm * vm, dj_infusion * infusion)
{
	int count = 0;
	dj_infusion *tail = vm->infusions;

	while (tail != NULL)
	{
		if (tail == infusion)
			return count;
		tail = tail->next;
		count++;
	}

	return 0;
}

/**
 * Gets an infusion by index.
 * @param vm the virtual machine to get a loaded infusion from
 * @param index the index of the infusion
 * @return the requested infusion, or NULL if index out of range.
 */
dj_infusion * dj_vm_getInfusion(dj_vm *vm, int index)
{
	int count = 0;
	dj_infusion *tail = vm->infusions;

	while (tail!=NULL)
	{
		if (index==count)
			return tail;
		tail = tail->next;
		count++;
	}

	return NULL;
}

/**
 * Gets a loaded infusion for a given infusion name.
 * @param vm the virtual machine to lookup the infusion in
 * @param name the name of the infusion. Has to be a NULL-terminated ascii string in program memory.
 * @return the infusion for the given name, or NULL if no such infusion is found
 */
dj_infusion *dj_vm_lookupInfusion(dj_vm *vm, dj_di_pointer name)
{
	dj_infusion *finger = vm->infusions;

	while (finger!=NULL)
	{
		if (dj_di_strEquals(dj_di_header_getInfusionName(finger->header), name)) return finger;
		finger = finger->next;
	}
	return NULL;
}

/**
 * Loads an infusion into the virtual machine.
 * @param vm the virtual machine object to load the infusion into
 * @param di a di pointer to the infusion file in program space
 * @return a newly loaded infusion, or NULL in case of fail
 */
dj_infusion *dj_vm_loadInfusion(dj_vm *vm, dj_di_pointer di, dj_named_native_handler native_handlers[], unsigned char numHandlers)
{
	int i;
	dj_infusion *infusion;
	dj_di_pointer element;
	dj_di_pointer staticFieldInfo = DJ_DI_NOT_SET;
	dj_di_pointer infusionList = DJ_DI_NOT_SET;
	dj_thread * thread;
	dj_global_id entryPoint;

	// iterate over the child elements, and find the static
	// field size info block. We need this info to allocate
	// the memory to hold the static fields for this
	// infusion
	for (i=0; i<dj_di_getListSize(di); i++)
	{
		element = dj_di_getListElement(di, i);
		switch (dj_di_element_getId(element))
		{
		case STATICFIELDINFO:
			staticFieldInfo = element;
			break;
		case INFUSIONLIST:
			infusionList = element;
			break;
		}
	}

	// Check if each of the required elements were found
	if (staticFieldInfo==DJ_DI_NOT_SET||infusionList==DJ_DI_NOT_SET)
		dj_panic(DJ_PANIC_MALFORMED_INFUSION);

	// allocate the Infusion struct
	infusion = dj_infusion_create(staticFieldInfo, dj_di_infusionList_getSize(infusionList));

	// if we're out of memory, let the caller handle it
	if (infusion==NULL) return NULL;

	// iterate over the child elements and get references
	// to the class list and method implementation list,
	// and get the header

	for (i=0; i<dj_di_getListSize(di); i++)
	{
		element = dj_di_getListElement(di, i);
		switch (dj_di_element_getId(element))
		{
		case HEADER:
			infusion->header = element;
			break;
		case CLASSLIST:
			infusion->classList = element;
			break;
		case METHODIMPLLIST:
			infusion->methodImplementationList = element;
			break;
		case STRINGTABLE:
			infusion->stringTable = element;
			break;
		}

	}

	// Check if each of the required elements was found
	if (infusion->stringTable==DJ_DI_NOT_SET||infusion->classList==DJ_DI_NOT_SET||infusion->methodImplementationList==DJ_DI_NOT_SET||infusion->header==DJ_DI_NOT_SET)
		dj_panic(DJ_PANIC_MALFORMED_INFUSION);

	// Check if the infusion is of a compatible type (since different branches use different infusion formats,
	// and forgetting to recompile the infuser is a potentially hard to find bug).
	if (dj_di_header_getInfusionFormatVersion(infusion->header) != INFUSION_FORMAT_VERSION)
		dj_panic(DJ_PANIC_INFUSION_VERSION_MISMATCH);

	// iterate over the referenced infusion list and set the appropriate pointers
	for (i=0; i<dj_di_infusionList_getSize(infusionList); i++)
	{
		dj_di_pointer name = dj_di_infusionList_getChild(infusionList, i);
		dj_infusion *referenced_infusion = dj_vm_lookupInfusion(vm, name);

		if (infusion==NULL)
			dj_panic(DJ_PANIC_UNSATISFIED_LINK);

		infusion->referencedInfusions[i] = referenced_infusion;
	}

	// add the new infusion to the VM
	dj_vm_addInfusion(vm, infusion);

	// We're assuming here that base.di is the first file in the archive
	if (vm->systemInfusion == NULL)
		vm->systemInfusion = infusion;

	// This code was originally in load dj_vm_loadInfusionArchive.
	// Moved here because the application is not in an archive, but needs
	// some of the same code (not native_handlers, but class initialisers
	// and creating a thread)
#ifdef DARJEELING_DEBUG
	char name[64];

	dj_infusion_getName(infusion, name, 64);

	DEBUG_LOG(DBG_DARJEELING, "Loaded infusion %s.\n", name);
#endif

	for (i=0; i<numHandlers; i++)
	{

		if (dj_di_strEqualsDirectStr(dj_di_header_getInfusionName(infusion->header), native_handlers[i].name))
		{
			infusion->native_handlers = native_handlers[i].handlers;

#ifdef DARJEELING_DEBUG
			DEBUG_LOG(DBG_DARJEELING, "Attached native handler to infusion %s.\n", name);
#endif
		}
	}

	// run class initialisers for this infusion
	infusion = dj_vm_runClassInitialisers(vm, infusion);

	// find the entry point for the infusion
	if ((entryPoint.entity_id=dj_di_header_getEntryPoint(infusion->header))!=255)
	{
		// create a new thread and add it to the VM
		entryPoint.infusion = infusion;
		thread = dj_thread_create_and_run(entryPoint);

		if (thread==NULL)
	        dj_panic(DJ_PANIC_OUT_OF_MEMORY);

		dj_vm_addThread(vm, thread);
	}

	return infusion;
}

/**
 * Checks wheter it's safe to unload an infusion. Unloading an infusion is unsafe if
 * another loaded infusion imports it. If you want to unload the infusion you *must*
 * unload the infusions that depend on it first, or leave the VM in an undefined state.
 * @param vm the virtual machine context
 * @param unloadInfusion the infusion to unload
 */
bool dj_vm_safeToUnload(dj_vm *vm, dj_infusion * unloadInfusion)
{
	dj_infusion * infusion;

	infusion = vm->infusions;
	while (infusion!=NULL)
	{

		// dj_infusion_getReferencedInfusionIndex returns 0 is the unloadInfusion is equal to
		// infusion, a positive number if infusion imports unloadInfusion, and -1 otherwise.
		// Therefore this check is for a positive number, meaning that infusion imports
		// unloadInfusion, making it unsafe to unload that infusion.
		if (dj_infusion_getReferencedInfusionIndex(infusion, unloadInfusion)>0)
			return false;

		infusion = infusion->next;
	}

	return true;
}

/**
 * Unloads an infusion. Make sure you check whether it's safe to do so using dj_vm_safeToUnload! If you
 * unload an infusion that another infusion depends on you'll leave the VM in an undefined state. Also,
 * a kitten will die.
 * @param vm the virtual machine context
 * @param unloadInfusion the infusion to unload
 */
void dj_vm_unloadInfusion(dj_vm *vm, dj_infusion * unloadInfusion)
{
	dj_thread * thread;
	dj_frame * frame;
	dj_infusion * infusion;
	dj_infusion * prev;
	int index;

	// kill any thread that is currently executing any method in the infusion that we're unloading
	thread = vm->threads;
	while (thread!=NULL)
	{

		// check each frame
		frame = thread->frameStack;
		while (frame!=NULL)
		{
			if (frame->method.infusion==unloadInfusion)
			{
				// kill the thread
				thread->status = THREADSTATUS_FINISHED;

				break;
			}
			frame = frame->parent;
		}

		// next thread
		thread = thread->next;
	}

	// shift runtime IDs
	dj_mem_shiftRuntimeIDs(unloadInfusion->class_base, dj_di_parentElement_getListSize(unloadInfusion->classList));

	// update other infusions
	infusion = vm->infusions;
	while (infusion!=NULL)
	{
		if (infusion->class_base>unloadInfusion->class_base)
			infusion->class_base -= dj_di_parentElement_getListSize(unloadInfusion->classList);

		infusion = infusion->next;
	}

	prev = NULL;

	index = dj_vm_getInfusionId(vm, unloadInfusion);

	// remove the infusion from the list
	if (index==0)
	{
		vm->infusions = unloadInfusion->next;
	} else
	{
		prev = dj_vm_getInfusion(vm, index - 1);
		prev->next = unloadInfusion->next;
	}

}

/**
 * Enumerate the files in an ar archive and load the infusions inside.
 * @param archive.start a pointer in program memory to the start of the archive
 * @param archive.end a pointer in program memory to the end of the archive
 * @param native_handlers a list of named native handlers to hook into infusions as they are loaded
 */
#define AR_FHEADER_SIZE 8
#define AR_EHEADER_SIZE 60
#define AR_EHEADER_SIZE_START 48
#define AR_EHEADER_SIZE_END 58

typedef struct
{
        dj_di_pointer start;
        dj_di_pointer end;
} dj_archive;

void dj_vm_loadInfusionArchive(dj_vm * vm, dj_di_pointer archive, dj_named_native_handler native_handlers[], unsigned char numHandlers)
{
	for (uint8_t i=0; i<dj_archive_number_of_files(archive); i++) {
		dj_di_pointer file = dj_archive_get_file(archive, i);
		if (dj_archive_filetype(file) == DJ_FILETYPE_LIB_INFUSION) {
			dj_infusion * infusion = dj_vm_loadInfusion(vm, file, native_handlers, numHandlers);
			if (infusion == NULL) {
				DARJEELING_PRINTF("Not enough space to create the infusion nr %d in archive.\n", i);
		        dj_panic(DJ_PANIC_OUT_OF_MEMORY);
		    }
		}
	}
}

/**
 * Gets the runtime class id for a class in the system infusion.
 * TODO: shouldn't this just return entity_id, since the system infusion is always the first loaded?
 * @param vm the virtual machine context
 * @param entity_id the entity_id of the class in question
 * @return the runtime Id of the given class
 */
uint8_t dj_vm_getSysLibClassRuntimeId(dj_vm *vm, uint8_t entity_id)
{
	dj_global_id class_id = (dj_global_id){vm->systemInfusion, entity_id};
	return dj_global_id_getRuntimeClassId(class_id);
}

/**
 * Creates an instance of a class in the system infusion.
 * @param vm the virtual machine context
 * @param entity_id the entity_id of the class to instantiate
 * @return a new class instance or NULL if fail (out of memory)
 */
dj_object * dj_vm_createSysLibObject(dj_vm *vm, uint8_t entity_id)
{
	dj_global_id class_id = (dj_global_id){vm->systemInfusion, entity_id};
	uint8_t runtime_id = dj_global_id_getRuntimeClassId(class_id);
	dj_di_pointer classDef = dj_vm_getRuntimeClassDefinition(vm, runtime_id);
	return dj_object_create(runtime_id,
			dj_di_classDefinition_getNrRefs(classDef),
			dj_di_classDefinition_getOffsetOfFirstReference(classDef)
			);
}

/**
 * Adds a thread to a virtual machine.
 * @param vm the virtual machine context
 * @param thread the thread to add to the vm
 */
void dj_vm_addThread(dj_vm *vm, dj_thread *thread)
{
	dj_thread *tail = vm->threads;
	while ((tail!=NULL)&&(tail->next!=NULL))
		tail = tail->next;

	if (tail==NULL)
	{
		// list is empty, add as first element
		thread->id = 0;
		vm->threads = thread;
	}
	else
	{
		// add to the end of the list
		thread->id = tail->id + 1;
		tail->next = thread;
	}

	// The new thread is the last element so its next should be NULL.
	thread->next = NULL;
}

/**
 * Removes a thread from a virtual machine.
 * @param vm the virtual machine context
 * @param thread the thread to add to the vm
 */
void dj_vm_removeThread(dj_vm *vm, dj_thread *thread)
{

	if (vm->threads==thread)
	{
		vm->threads = thread->next;
	} else
	{
		dj_thread *pre = vm->threads;
		while ( (pre!=NULL) && (pre->next!=thread) )
			pre = pre->next;

		pre->next = thread->next;
	}

}

/**
 * Counts the threads in a virtual machine.
 * @param vm the virtual machine context
 * @return the number of threads in the virtual machine
 */
int dj_vm_countThreads(dj_vm *vm)
{
	int ret = 0;
	dj_thread *tail = vm->threads;

	while (tail!=NULL)
	{
		tail = tail->next;
		ret++;
	}

	return ret;
}

/**
 * Wakes threads that are sleeping or blocked.
 * @param vm the virtual machine context
 */
void dj_vm_wakeThreads(dj_vm *vm)
{
	dj_thread *thread = vm->threads;
	dj_monitor * monitor;

	dj_time_t time = dj_timer_getTimeMillis();

	while (thread!=NULL)
	{
		// wake sleeping threads
		if (thread->status==THREADSTATUS_SLEEPING)
			if (thread->scheduleTime <= time)
				thread->status=THREADSTATUS_RUNNING;

		// wake waiting threads that timed out
		if (thread->status==THREADSTATUS_WAITING_FOR_MONITOR)
			if ((thread->scheduleTime!=0)&&((int32_t)thread->scheduleTime <= (int32_t)time))
			{
				thread->status=THREADSTATUS_RUNNING;
				thread->monitorObject=NULL;
			}

		// wake blocked threads
		if (thread->status==THREADSTATUS_BLOCKED_FOR_MONITOR)
		{
			monitor = dj_vm_getMonitor(vm, thread->monitorObject);
			if (monitor->count==0)
			{
				monitor->waiting_threads--;

				monitor->count = 1;
				monitor->owner = thread;

				thread->status = THREADSTATUS_RUNNING;
			}

		}

		thread = thread->next;
	}

}

/**
 * Notifes one or more threads waiting for a lock.
 * @param vm the virtual machine context
 * @param object the lock object
 * @param all whether or not to wake all threads waiting for this lock, or just one
 */
void dj_vm_notify(dj_vm *vm, dj_object *object, bool all)
{
	dj_thread *thread = vm->threads;
	dj_thread *next;

	while (thread != NULL)
	{
		next = thread->next;

		if ((thread->status == THREADSTATUS_WAITING_FOR_MONITOR)&&
			(thread->monitorObject==object))
		{
			thread->monitorObject = NULL;
			thread->status = THREADSTATUS_RUNNING;
			if (!all) return;
		}

		thread = next;
	}
}

/**
 * Checks for any threads that have reached the THREADSTATUS_FINISHED state, and removes them from
 * the virtual machine context.
 * @param vm the virtual machine context
 */
void dj_vm_checkFinishedThreads(dj_vm *vm)
{
	dj_thread *thread = vm->threads;
	dj_thread *next;

	while (thread != NULL) {
		next = thread->next;
		if (thread->status == THREADSTATUS_FINISHED)
		{
			if (vm->currentThread == thread)
				vm->currentThread = NULL;
			dj_vm_removeThread(vm, thread);
			dj_thread_destroy(thread);
		}

		thread = next;
	}

}

/**
 * Gets a thread from the virtual machine by index. The index is not equal to the thread ID.
 * @param vm the virtual machine context
 * @param index the index of the thread
 * @return the requested thread, or NULL if the index is out of bounds
 */
dj_thread * dj_vm_getThread(dj_vm * vm, int index)
{
	dj_thread *finger;
	int i=0;

	finger = vm->threads;
	while ((index>i)&&finger)
	{
		finger = finger->next;
		i++;
	}

	return finger;

}

/**
 * Gets a thread by its ID. Used by the native methods of the java.lang.Thread object to control threads and
 * call yield, sleep, wait, etc.
 * @param vm the virtual machine context
 * @param id the ID of the desired Thread
 */
dj_thread * dj_vm_getThreadById(dj_vm * vm, int id)
{
	dj_thread *finger;

	finger = vm->threads;
	while (finger!=NULL)
	{
		if (finger->id==id)
			return finger;

		finger = finger->next;
	}

	return NULL;
}


/**
 * Counts the number of threads that are 'live', meaning that they are either running now or will
 * potentially start running in the future. This includes blocked and sleeping threads.
 * @param vm the virtual machine context
 */
int dj_vm_countLiveThreads(dj_vm *vm)
{
	int ret = 0;
	dj_thread *thread = vm->threads;

	while (thread!=NULL)
	{
		if ( (thread->status==THREADSTATUS_RUNNING) ||
				(thread->status==THREADSTATUS_BLOCKED) ||
				(thread->status==THREADSTATUS_BLOCKED_FOR_MONITOR) ||
				(thread->status==THREADSTATUS_WAITING_FOR_MONITOR) ||
				(thread->status==THREADSTATUS_SLEEPING) ||
				(thread->status==THREADSTATUS_BLOCKED_FOR_IO)
				) ret++;
		thread = thread->next;
	}
	return ret;
}

dj_time_t dj_vm_getVMSleepTime(dj_vm * vm)
{
	dj_time_t scheduleTime, time, ret=-1;
	dj_thread *thread = vm->threads;

	time = dj_timer_getTimeMillis();

	while (thread!=NULL)
	{
		if (thread->status==THREADSTATUS_RUNNING) ret = 0;

		if (thread->status==THREADSTATUS_SLEEPING)
		{
			scheduleTime = thread->scheduleTime - time;
			if (scheduleTime<0) scheduleTime = 0;
			if (ret==-1||ret>scheduleTime) ret = scheduleTime;
		}

		thread = thread->next;
	}

	return ret;
}


/**
 * Wakes up any threads that need to be woken up and chooses the next thread to be started and
 * schedules it for execution. A simple round-robin scheme is used for thread selection.
 * @param vm the virtual machine context
 */
char dj_vm_schedule(dj_vm *vm)
{
	int maxPriority;

	dj_thread *thread, *selectedThread;

	// prune finished threads
	dj_vm_checkFinishedThreads(vm);

	// wake up any threads that need to be woken up
	dj_vm_wakeThreads(vm);

	// Simple round-robin scheduling algo:
	// Select the thread with the highest priority and set its priority to zero, and
	// increase the priorities of all other threads by one.
	selectedThread = NULL;
	thread = vm->threads;
	maxPriority = -1;
	while (thread!=NULL)
	{
		if (thread->status==THREADSTATUS_RUNNING)
		{
			thread->priority++;
			if (maxPriority<thread->priority)
			{
				selectedThread=thread;
				maxPriority=thread->priority;
			}
		}
		thread = thread->next;
	}

	char ret = dj_vm_activateThread(vm, selectedThread);
	return ret;
}

/**
 * Activates a thread, meaning that the current thread (if any) is switched out and the
 * new thread is switched in. Note that this method may be called with NULL, if there is
 * currently no active thread to run.
 * @param vm the virtual machine context
 * @param selectedThread the thread to activate
 */
char dj_vm_activateThread(dj_vm *vm, dj_thread *selectedThread)
{

	// stop the current thread
	if (vm->currentThread != NULL)
		dj_exec_deactivateThread(vm->currentThread);

	vm->currentThread = selectedThread;

	// check if we found a thread we can activate
	if (selectedThread!=NULL)
	{
		selectedThread->priority=0;
		dj_exec_activate_thread(selectedThread);
		return 1;
	} else
	{
		// no threads to schedule, return false
		return 0;
	}

}

/**
 * Updates pointers after the new offsets of objects have been calculated, and before the objects
 * are actually moved to their new locations.
 * @param vm the virtual machine context
 */
void dj_vm_mem_updatePointers(dj_vm *vm)
{
	vm->currentThread = dj_mem_getUpdatedPointer(vm->currentThread);
	vm->infusions = dj_mem_getUpdatedPointer(vm->infusions);
	vm->monitors = dj_mem_getUpdatedPointer(vm->monitors);
	vm->systemInfusion = dj_mem_getUpdatedPointer(vm->systemInfusion);
	vm->threads = dj_mem_getUpdatedPointer(vm->threads);
}


/**
 * Finds a monitor for the given object. If no monitor exists, one is created.
 * @param vm virtual machine context
 * @param object object that acts as a lock
 */
dj_monitor * dj_vm_getMonitor(dj_vm *vm, dj_object * object)
{
	int i;
	dj_monitor *monitor, *ret = NULL;
	dj_monitor_block *block, *newBlock;

	dj_mem_addSafePointer((void**)&object);

	// search for the monitor
	block = vm->monitors;
	for (i=0; i<vm->numMonitors; i++)
	{
		// check if the monitor applies to the given object
		monitor = &(block->monitors[i%MONITOR_BLOCK_SIZE]);

		if (monitor->object==object)
		{
			ret = monitor;
			break;
		}

		// if we have reached the end of a block, move to the next one
		if ((i%MONITOR_BLOCK_SIZE)==(MONITOR_BLOCK_SIZE-1))
			block = block->next;
	}

	// If the monitor is not yet in the monitor list, add the monitor to the end of the list.
	if (ret==NULL)
	{

		// check if we need to add a new block
		if ((vm->numMonitors%MONITOR_BLOCK_SIZE)==0)
		{
			// allocate a new monitor block
			newBlock = dj_monitor_block_create();

			// check for out of memory and let the caller deal with it
			if (newBlock==NULL) {
				dj_mem_removeSafePointer((void**)&object);
				return NULL;
			}

			// find the last block (do this after the allocation to make sure everything goes right
			// in case the allocation triggers garbage collection)
			for (block=vm->monitors; (block!=NULL)&&(block->next!=NULL); block=block->next);

			// add the new monitor to the virtual machine
			if (vm->monitors==NULL)
				vm->monitors = newBlock;
			else
				block->next = newBlock;

			block = newBlock;
		} else
			// find last block
			for (block=vm->monitors; (block!=NULL)&&(block->next!=NULL); block=block->next);

		// Create a new monitor and add it to the list.
		// note that vm->monitors acts like a stack, so vm->monitors points to the
		// block that was added last
		ret = &(block->monitors[vm->numMonitors%MONITOR_BLOCK_SIZE]);

		// reset the monitor to count=0, waiting_threads=0
		ret->count = 0;
		ret->waiting_threads = 0;
		ret->object = object;
		ret->owner = NULL;
		vm->numMonitors++;
		block->count++;

	}

	dj_mem_removeSafePointer((void**)&object);

	return ret;
}

void dj_vm_removeMonitor(dj_vm *vm, dj_monitor * monitor)
{
	int i;
	dj_monitor_block *block, *lastBlock;

	// search for the monitor
	block = vm->monitors;
	for (i=0; i<vm->numMonitors; i++)
	{
		if (&(block->monitors[i%MONITOR_BLOCK_SIZE])==monitor) break;

		// if we have reached the end of a block, move to the next one
		if ((i%MONITOR_BLOCK_SIZE)==(MONITOR_BLOCK_SIZE-1))
			block = block->next;
	}

	// find the last block
	for (lastBlock=vm->monitors; (lastBlock!=NULL)&&(lastBlock->next!=NULL); lastBlock=lastBlock->next);

	// reduce the monitor count
	vm->numMonitors--;
	lastBlock->count--;

	if (i<vm->numMonitors)
	{
		// remove the monitor by replacing it with the last monitor in the linked list
		block->monitors[i%MONITOR_BLOCK_SIZE] = lastBlock->monitors[vm->numMonitors%MONITOR_BLOCK_SIZE];
	}

	// check if we need to dealloc the last block
	if (vm->numMonitors%MONITOR_BLOCK_SIZE==0)
	{
		if (vm->numMonitors>0)
		{
			// find the block that comes just before the last block
			for (block=vm->monitors; (block!=lastBlock)&&(block->next!=lastBlock); block=block->next);
			// set its next pointer to NULL
			block->next = NULL;
		} else
		{
			vm->monitors = NULL;
		}

		// free the last block
		dj_mem_free(lastBlock);
	}

}


inline dj_global_id dj_vm_getRuntimeClass(dj_vm *vm, runtime_id_t id)
{
	dj_global_id ret;
	dj_infusion *infusion = vm->infusions;
	runtime_id_t base = 0;

	// TODO: optimize this! (binary search?)
	// TODO: test for multiple loaded infusions
	while (infusion!=NULL)
	{
		base = infusion->class_base;
		if ((id>=base)&&(id<base + dj_di_parentElement_getListSize(infusion->classList)))
		{
			ret.infusion = infusion;
			ret.entity_id = id - base;

			return ret;
		}
		infusion = infusion->next;
	}

	// TODO raise error, class not found
	DEBUG_LOG(DBG_DARJEELING, "error: class not found: %d\n", id);
	DARJEELING_PRINTF("error: class not found: %d\n", id);
#ifdef DARJEELING_DEBUG_FRAME
	dj_exec_debugCurrentFrame();
#endif
    dj_panic(DJ_PANIC_ILLEGAL_INTERNAL_STATE);

    // dead code to make compiler happy
    ret.entity_id=255;
    ret.infusion=NULL;
    return ret;
}

dj_di_pointer dj_vm_getRuntimeClassDefinition(dj_vm *vm, runtime_id_t id)
{
	dj_global_id global_id = dj_vm_getRuntimeClass(vm, id);
	return dj_infusion_getClassDefinition(global_id.infusion, global_id.entity_id);
}

dj_infusion *dj_vm_getSystemInfusion(dj_vm *vm)
{
	return vm->systemInfusion;
}

/**
 * Runs the class initialisers of a given infusion. The class initialisers are the
 * methods with the name <cinit>, that are generated for code inside <code>static { ... }</code>
 * and for statements like <code>static int foo = 1;</code>.
 * @param vm the virtual machine context
 */
// TODO niels clean up this function
dj_infusion* dj_vm_runClassInitialisers(dj_vm *vm, dj_infusion *infusion)
{
	int i, threadId;
	dj_thread * thread;
	dj_frame * frame;
	dj_global_id methodImplId;
	uint8_t infusionId;

	// store infusion ID so that we can get an up-to-date infusion pointer later
	infusionId = dj_vm_getInfusionId(vm, infusion);

	// create a new thread object to run the <CLINIT> methods in
	thread = dj_thread_create();

	infusion = dj_vm_getInfusion(vm, infusionId);

	if (thread == NULL)
	{
		DARJEELING_PRINTF("Not enough space for class initializer in infusion %s\n", (char *) dj_di_header_getInfusionName(infusion->header));
		dj_panic(DJ_PANIC_OUT_OF_MEMORY);
	}

	dj_vm_addThread(dj_exec_getVM(), thread);
	threadId = thread->id;

	// iterate over the class list and execute any class initialisers that are encountered
	int size = dj_di_parentElement_getListSize(infusion->classList);
	for (i=0; i<size; i++)
	{
		infusion = dj_vm_getInfusion(dj_exec_getVM(), infusionId);

		dj_di_pointer classDef = dj_di_parentElement_getChild(infusion->classList, i);
		methodImplId.entity_id = dj_di_classDefinition_getCLInit(classDef);
		methodImplId.infusion = infusion;

		if (methodImplId.entity_id!=255)
		{
			// create a frame to run the initialiser in
			methodImplId.infusion = infusion;
			frame = dj_frame_create(methodImplId);

			// if we're out of memory, panic
		    if (frame==NULL)
		    {
		        DEBUG_LOG(DBG_DARJEELING, "dj_vm_runClassInitialisers: could not create frame. Panicking\n");
		        DARJEELING_PRINTF("Not enough space to create a frame\n");
		        dj_panic(DJ_PANIC_OUT_OF_MEMORY);
		    }

		    // the thread we're running the class initialisers in.
		    thread = dj_vm_getThreadById(dj_exec_getVM(), threadId);
		    thread->frameStack = frame;
		    thread->status = THREADSTATUS_RUNNING;
			dj_exec_activate_thread(thread);

			// execute the method
			while (dj_vm_getThreadById(dj_exec_getVM(), threadId)->status!=THREADSTATUS_FINISHED)
			{
				// running the CLINIT method may trigger garbage collection
				dj_exec_run(RUNSIZE);
			}
		}
	}

	// clean up the thread
	thread = dj_vm_getThreadById(dj_exec_getVM(), threadId);
	dj_vm_removeThread(vm, thread);
	dj_thread_destroy(thread);
	vm->currentThread = NULL;
	return infusion;
}

