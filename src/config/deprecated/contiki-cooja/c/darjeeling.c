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
#include "stdlib.h"
#include "string.h"
#include "main.h"
#include "heap.h"

#include "jlib_base.h"
#include "jlib_darjeeling.h"
#include "jlib_radio.h"
#include "pointerwidth.h"

#ifdef IS_COOJA
#include "node-id.h"
static unsigned char *mem;
#else
static unsigned char mem[HEAPSIZE];
#endif

#if ASSEMBLY_DEBUGGING
FILE* logFile;
#endif

extern unsigned char di_archive_data[];
extern size_t di_archive_size;

static dj_vm *vm;

extern int16_t TOS_NODE_ID;
char * ref_t_base_address;

void dj_init()
{
#ifdef IS_COOJA
#if ASSEMBLY_DEBUGGING
	char name[100];
	sprintf(name, "file%d", node_id);
	logFile= fopen(name, "w");
	if (logFile == NULL)
		printf("File is not opened\n");

	fprintf(logFile, "Log file for node %d\n", node_id);
#endif
	// initialise memory manager

	mem = malloc(HEAPSIZE);
#endif


	dj_mem_init(mem, HEAPSIZE);
    ref_t_base_address = (char*)mem - 42;


	// initialise timer
	dj_timer_init();

#ifdef HAS_USART
	rs232_init(RS232_PORT_0, USART_BAUD_57600, USART_PARITY_NONE | USART_DATA_BITS_8 | USART_STOP_BITS_1);
	rs232_redirect_stdout(RS232_PORT_0);
#endif
	// create a new VM
	vm = dj_vm_create();

	// tell the execution engine to use the newly created VM instance
	dj_exec_setVM(vm);

	dj_named_native_handler handlers[] = {
		{ "base", &base_native_handler },
		{ "darjeeling", &darjeeling_native_handler },
		{ "radio", &radio_native_handler}
	};
	int length = sizeof(handlers)/ sizeof(handlers[0]);
        dj_archive archive;
        archive.start = (dj_di_pointer)di_archive_data;
        archive.end = (dj_di_pointer)(di_archive_data + di_archive_size);

        dj_vm_loadInfusionArchive(vm, &archive, handlers, length);      // load the embedded infusions

	leds_init();

/*
	// pre-allocate an OutOfMemoryError object
	dj_object *obj = dj_vm_createSysLibObject(vm, BASE_CDEF_java_lang_OutOfMemoryError);
	vm_mem_setPanicExceptionObject(obj);
*/


}

void dj_run()
{
	if (dj_vm_countLiveThreads(vm)>0)
	{
		// find thread to schedule
		dj_vm_schedule(vm);

		if (vm->currentThread!=NULL)
			if (vm->currentThread->status==THREADSTATUS_RUNNING)
				dj_exec_run(RUNSIZE);
	}
//	return dj_vm_getVMSleepTime(vm);

}

void notify_radio_receive();
void notify_radio_sendDone();

/**
 * this notify method is used to inform the receive thread that
 * 'the send thread' has received something. The definition looks a bit nonsense,
 * but in fact, each send thread has a callback method in that the thread is responsible
 * for receiving messages. When a message is received, receive thread should start
 * reading the message
 */
void dj_notifyRadioReceive()
{
	notify_radio_receive();
}
/**
 * this notify method is used to inform the receive thread that
 * 'the send thread' has received something. The definition looks a bit nonsense,
 * but in fact, each send thread has a callback method in that the thread is responsible
 * for receiving messages. When a message is received, receive thread should start
 * reading the message
 */
void dj_notifyRadioSendDone()
{
	notify_radio_sendDone();
}

