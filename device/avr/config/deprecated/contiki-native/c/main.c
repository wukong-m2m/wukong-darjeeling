/*
 * main.c
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
 
#include "contiki.h"
#include "dev/leds.h"

#include "stdio.h"

#include "debug.h"
#include "vm.h"
#include "heap.h"
#include "infusion.h"
#include "types.h"
#include "vmthread.h"
#include "djtimer.h"
#include "execution.h"

#ifdef HAS_USART
#include "dev/rs232.h"
#endif

#include "jlib_base.h"
#include "jlib_darjeeling2.h"

#include "pointerwidth.h"
char * ref_t_base_address;

/*
extern char * * _binary_infusions_start;
extern char * * _binary_infusions_end;
extern size_t * _binary_infusions_size;
*/

extern unsigned char di_archive_data[];
extern size_t di_archive_size;

/*---------------------------------------------------------------------------*/
PROCESS(blink_process, "Darjeeling");
AUTOSTART_PROCESSES(&blink_process);
/*---------------------------------------------------------------------------*/

static unsigned char mem[MEMSIZE];
static struct etimer et;
static dj_vm * vm;

char *ref_t_base_address;

PROCESS_THREAD(blink_process, ev, data)
{
	PROCESS_EXITHANDLER(goto exit;)
	PROCESS_BEGIN();

	// initialise memory manager
	dj_mem_init(mem, MEMSIZE);
	ref_t_base_address = (char*)mem - 42;

	// initialise timer
	dj_timer_init();

	// init hw
	leds_init();

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
		{ "darjeeling2", &darjeeling2_native_handler }
	};

	/*
	dj_vm_loadInfusionArchive(vm, (dj_di_pointer)&_binary_infusions_start, (dj_di_pointer)&_binary_infusions_end, handlers, 2);
	*/
	int length = sizeof(handlers)/ sizeof(handlers[0]);
        dj_archive archive;
        archive.start = (dj_di_pointer)di_archive_data;
        archive.end = (dj_di_pointer)(di_archive_data + di_archive_size);

        dj_vm_loadInfusionArchive(vm, &archive, handlers, length);      // load the embedded infusions

	printf("%d infusions loaded\n", dj_vm_countInfusions(vm));

// load the embedded infusions
	// dj_loadEmbeddedInfusions(vm);

	while (true)
	{

		// start the main execution loop
		if (dj_vm_countLiveThreads(vm)>0)
		{
			dj_vm_schedule(vm);

			if (vm->currentThread!=NULL)
				if (vm->currentThread->status==THREADSTATUS_RUNNING)
					dj_exec_run(RUNSIZE);

		}

		// can't get PROCESS_YIELD to work, quick hack to wait 1 clock tick
	    etimer_set(&et, 1);
	    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	}

exit:
	leds_off(LEDS_ALL);
	PROCESS_END();
}

