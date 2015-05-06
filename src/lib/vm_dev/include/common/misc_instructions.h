/*
 * misc_instructions.h
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
 

#ifndef __misc_instructions_h
#define __misc_instructions_h

#include <string.h>

#include "jstring.h"

#include "jlib_base.h"

static inline void LDS()
{
	dj_local_id localStringId = dj_fetchLocalId();
	ref_t obj = DO_LDS(localStringId);
	if (obj!=0)
		pushRef(obj);
}

static inline void NEW()
{
	dj_local_id dj_local_id = dj_fetchLocalId();
	ref_t obj = DO_NEW(dj_local_id);
	if (obj!=0)
		pushRef(obj);
}

static inline void INSTANCEOF()
{
	dj_local_id localClassId = dj_fetchLocalId();
	ref_t ref = popRef();
	pushShort(DO_INSTANCEOF(localClassId, ref));
}

static inline void CHECKCAST()
{
	dj_local_id classLocalId = dj_fetchLocalId();
	ref_t ref = peekRef();

	dj_object * object = REF_TO_VOIDP(ref);

	// NULL passes checkcast
	if(object==NULL)
		return;

	if (dj_object_getRuntimeId(object) == CHUNKID_INVALID)
	{
		dj_exec_createAndThrow(BASE_CDEF_javax_darjeeling_vm_ClassUnloadedException);
		return;
	}

	if ( !dj_global_id_testType(object, classLocalId) )
		dj_exec_createAndThrow(BASE_CDEF_java_lang_ClassCastException);
}

static inline void MONITORENTER()
{
    dj_monitor * monitor;
    dj_object * obj;

	ref_t objRef = dj_exec_stackPopRef();

	// check for null pointer
	if (objRef==nullref)
	{
		dj_exec_createAndThrow(BASE_CDEF_java_lang_NullPointerException);
		return;
	}

	obj = (dj_object*)REF_TO_VOIDP(objRef);
	dj_mem_addSafePointer((void**)&obj);

    DEBUG_ENTER_NEST_LOG(DBG_DARJEELING, "MONITORENTER() thread:%d, object%p\n", dj_exec_getCurrentThread()->id, REF_TO_VOIDP(objRef));

	// get the monitor for this object
	monitor = dj_vm_getMonitor(dj_exec_getVM(), obj);

	// if the monitor didn't exist and could not be created, throw exception
	if (monitor==NULL)
	{
		dj_exec_createAndThrow(BASE_CDEF_java_lang_OutOfMemoryError);
	} else
	{

		// check if we can enter the monitor
		if (monitor->count==0)
		{
			DEBUG_LOG(DBG_DARJEELING, "Entering monitor %p\n",monitor);
			// we can enter the monitor, huzzaa
			monitor->count = 1;
			monitor->owner = dj_exec_getCurrentThread();

		} else {
			if (monitor->owner==dj_exec_getCurrentThread())
			{
				DEBUG_LOG(DBG_DARJEELING, "Reentering monitor %p. count is now %d\n",monitor,monitor->count+1);

				monitor->count++;
			} else
			{

				// we can't enter, so just block
				dj_exec_getCurrentThread()->status = THREADSTATUS_BLOCKED_FOR_MONITOR;
				dj_exec_getCurrentThread()->monitorObject = obj;
				monitor->waiting_threads++;
				DEBUG_LOG(DBG_DARJEELING, "monitor is already held by someone. let's block\n");

				dj_exec_breakExecution();
			}
		}

	}

    DEBUG_EXIT_NEST_LOG(DBG_DARJEELING, "MONITORENTER()\n");
	dj_mem_removeSafePointer((void**)&obj);

}

static inline void MONITOREXIT()
{
	dj_monitor * monitor;

	// Peek the object, don't pop it yet until after dj_vm_getMonitor is called.
	// This because dj_vm_getMonitor may require a memory allocation and thus may trigger
	// garbage compaction. By leaving the object on the operand stack for now we
	// are guaranteed that we can pop a valid reference after compaction.
	dj_object * obj = REF_TO_VOIDP(peekRef());


	// check if the object is still valid
	if (dj_object_getRuntimeId(obj)==CHUNKID_INVALID)
	{
		dj_exec_createAndThrow(BASE_CDEF_javax_darjeeling_vm_ClassUnloadedException);
		return;
	}

	// check for null pointer
	if (obj==NULL)
	{
		dj_exec_createAndThrow(BASE_CDEF_java_lang_NullPointerException);
		return;
	}

	DEBUG_ENTER_NEST_LOG(DBG_DARJEELING, "MONITOREXIT() thread:%d, object:%p\n", dj_exec_getCurrentThread()->id, obj);

    // find the monitor associated with the object
	monitor = dj_vm_getMonitor(dj_exec_getVM(), obj);

    // if the monitor wasn't found, raise an error
	if(monitor == NULL)
	{
		DEBUG_LOG(DBG_DARJEELING, "Monitor is NULL at MONITOREXIT\n");
		dj_exec_createAndThrow(BASE_CDEF_java_lang_VirtualMachineError);
		return;
	}

	// safely pop the object
	obj = REF_TO_VOIDP(popRef());

	dj_thread * thread = dj_exec_getCurrentThread();

	// exit the monitor
	monitor->count--;
	monitor->owner = NULL;

    DEBUG_LOG(DBG_DARJEELING, "Exiting monitor %p, count is now %d\n", monitor, monitor->count);

	// remove the monitor if the count has reached 0
    if ((monitor->count==0)&&(monitor->waiting_threads==0))
	{
        DEBUG_LOG(DBG_DARJEELING, "Removing monitor\n");
		dj_vm_removeMonitor(dj_exec_getVM(), monitor);
	}

	// clear thread's monitor object
    thread->monitorObject = NULL;

    DEBUG_EXIT_NEST_LOG(DBG_DARJEELING, "MONITOREXIT()\n");
}

#endif /* __misc_instructions_h */
