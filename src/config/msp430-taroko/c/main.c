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
 

#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "vm.h"
#include "heap.h"
#include "infusion.h"
#include "types.h"
#include "vmthread.h"
#include "djtimer.h"
#include "execution.h"
#include "hooks.h"
#include "core.h"

#include "pointerwidth.h"

extern dj_di_pointer di_lib_infusions_archive_data[];
// di_app_infusion_archive_data is declared in djarchive.h

// From GENERATEDlibinit.c, which is generated during build based on the libraries in this config's libs.
extern dj_named_native_handler java_library_native_handlers[];
extern uint8_t java_library_native_handlers_length;

unsigned char mem[HEAPSIZE];

int main()
{
	// TODONR How does this work on Taroko?
	// initialise serial port
	// avr_serialInit(115200);

	core_init(mem, HEAPSIZE);
	dj_vm_main(di_lib_infusions_archive_data, di_app_infusion_archive_data, java_library_native_handlers, java_library_native_handlers_length);

	// Listen to the radio
	while(true)
		dj_hook_call(dj_core_pollingHook, NULL);

	return 0;
}
