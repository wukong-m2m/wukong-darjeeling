/*
 * darjeeling.c
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
 
#include "darjeeling.h"
#include <stdlib.h>
#include <stdio.h>
#include "debug.h"
#include "vm.h"
#include "heap.h"
#include "infusion.h"
#include "types.h"
#include "vmthread.h"
#include "djtimer.h"
#include "execution.h"


#include "jlib_base.h"
#include "jlib_darjeeling.h"
#include "tosconfig.h"
#include "panic.h"
#include "tossim.h"


#ifdef WITH_RADIO
#include "jlib_radio.h"
#endif
extern unsigned char di_archive_data[];
extern size_t di_archive_size;

void dj_init()
{
	//before anything starts, define global variables
	struct tossim_global_variables* global_variables = malloc(sizeof(struct tossim_global_variables));
	struct tossim_UGLY_global_variables* UGLY_global_variables = malloc(sizeof(struct tossim_UGLY_global_variables));
	setGlobalVariables(global_variables);
	setUglyGlobalVariables(UGLY_global_variables);

	dj_vm *vm;
	unsigned char *mem = malloc(HEAPSIZE);
	// initialise memory manager
	dj_mem_init(mem, HEAPSIZE);
    _global_ref_t_base_address = (char*)mem - 42;

	// initialise timer
	dj_timer_init();

	// create a new VM
	vm = dj_vm_create();
	if (vm == nullref){
		//fail with a unknown type exception
    	dj_panic(-1);
	}

	// tell the execution engine to use the newly created VM instance
	dj_exec_setVM(vm);

	// load the embedded infusions

	dj_named_native_handler handlers[] = {
			{ "base", &base_native_handler },
			{ "darjeeling", &darjeeling_native_handler }
#ifdef WITH_RADIO
			,{ "radio", &radio_native_handler }
#endif
		};

	int length = sizeof(handlers)/ sizeof(handlers[0]);
        archive.start = (dj_di_pointer)di_archive_data;
        archive.end = (dj_di_pointer)(di_archive_data + di_archive_size);

        dj_vm_loadInfusionArchive(vm, &archive, handlers, length);      // load the embedded infusions


	DARJEELING_PRINTF("%d infusions loaded\n", dj_vm_countInfusions(dj_exec_getVM()));

	// pre-allocate an OutOfMemoryError object
	dj_object *obj = dj_vm_createSysLibObject(dj_exec_getVM(), BASE_CDEF_java_lang_OutOfMemoryError);
	vm_mem_setPanicExceptionObject(obj);
}

uint32_t dj_run()
{
	// find thread to schedule
	dj_vm_schedule(dj_exec_getVM());

	if (dj_exec_getCurrentThread()!=NULL)
		if (dj_exec_getCurrentThread()->status==THREADSTATUS_RUNNING){
			dj_exec_run(RUNSIZE);
		}
	return dj_vm_getVMSleepTime(dj_exec_getVM());

}

void notify_radio_receive();
void notify_radio_sendDone();

void dj_notifyRadioReceive()
{
#ifdef WITH_RADIO
	notify_radio_receive();
#endif
}

void dj_notifyRadioSendDone()
{
#ifdef WITH_RADIO
	notify_radio_sendDone();
#endif
}

