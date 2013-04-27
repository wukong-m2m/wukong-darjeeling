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
#include <stdbool.h>
#include <string.h>
#include <getopt.h>

#include "core.h"
#include "types.h"
#include "vm.h"
#include "hooks.h"
#include "djarchive.h"

#include "posix_utils.h"

// From GENERATEDlibinit.c, which is generated during build based on the libraries in this config's libs.
extern dj_named_native_handler java_library_native_handlers[];
extern uint8_t java_library_native_handlers_length;

int main(int argc,char* argv[])
{
	posix_parse_command_line(argc, argv);

	// Read the lib and app infusion archives from file
	char* di_lib_infusions_archive_data = posix_load_infusion_archive("lib_infusions.dja");
	char* di_app_infusion_archive_data = posix_load_infusion_archive("app_infusion.dja");

	// initialise memory manager
	void *mem = malloc(HEAPSIZE);
	ref_t_base_address = (char*)mem - 42;

	core_init(mem, HEAPSIZE);
	dj_vm_main((dj_di_pointer)di_lib_infusions_archive_data, (dj_di_pointer)di_app_infusion_archive_data, java_library_native_handlers, java_library_native_handlers_length);

	// Listen to the radio
	while(true)
		dj_hook_call(dj_core_pollingHook, NULL);

	return 0;
}
