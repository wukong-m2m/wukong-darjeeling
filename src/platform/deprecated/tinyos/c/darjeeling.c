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

#include "tosconfig.h"

#include "jlib_base.h"
#include "jlib_darjeeling2.h"

extern unsigned char di_archive_data[];
extern size_t di_archive_size;
static unsigned char mem[HEAPSIZE];
static dj_vm *vm;

void dj_init()
{

	// initialise memory manager
	dj_mem_init(mem, HEAPSIZE);

	// initialise timer
	dj_timer_init();

	// create a new VM
	vm = dj_vm_create();

	// tell the execution engine to use the newly created VM instance
	dj_exec_setVM(vm);

	// load the embedded infusions
	dj_named_native_handler handlers[] = {
			{ "base", &base_native_handler },
			{ "darjeeling2", &darjeeling2_native_handler }
        };

	int length = sizeof(handlers) / sizeof(handlers[0]);
        dj_archive archive;
        archive.start = (dj_di_pointer)di_archive_data;
        archive.end = (dj_di_pointer)(di_archive_data + di_archive_size);

	dj_vm_loadInfusionArchive(vm, &archive, handlers, length);	// load the embedded infusions

	// pre-allocate an OutOfMemoryError object
	dj_object *obj = dj_vm_createSysLibObject(vm, BASE_CDEF_java_lang_OutOfMemoryError);
	vm_mem_setPanicExceptionObject(obj);

}

dj_time_t dj_run()
{
	// find thread to schedule
	dj_vm_schedule(vm);

	if (dj_exec_getCurrentThread()!=NULL)
		if (dj_exec_getCurrentThread()->status==THREADSTATUS_RUNNING)
			dj_exec_run(RUNSIZE);

	return dj_vm_getVMSleepTime(vm);

}

void notify_radio_receive();
void notify_radio_sendDone();
//void notify_serial_sendDone();

void dj_notifyRadioReceive()
{
#ifdef TOS_RADIO
	notify_radio_receive();
#endif
}

void dj_notifyRadioSendDone()
{
#ifdef TOS_RADIO
	notify_radio_sendDone();
#endif
}

void dj_notifyUartReceiveByte(uint8_t byte)
{
	notify_serial_receiveByte(byte);
}
