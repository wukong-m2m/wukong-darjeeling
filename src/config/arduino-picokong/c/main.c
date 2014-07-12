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

#include "debug.h"
#include "heap.h"
#include "types.h"
#include "djtimer.h"
#include "djarchive.h"
#include "hooks.h"
#include "core.h"

#include "avr.h"
#include "wkpf.h"
#include "wkpf_main.h"
#include "wkpf_wuobjects.h"
#include "wkpf_links.h"


extern const unsigned char di_lib_infusions_archive_data[];
extern const unsigned char di_app_infusion_archive_data[];

// From GENERATEDlibinit.c, which is generated during build based on the libraries in this config's libs.
extern dj_named_native_handler java_library_native_handlers[];
extern uint8_t java_library_native_handlers_length;


unsigned char mem[2048];
void wukong_setup()
{
	// Declared in djarchive.c so that the reprogramming code can find it.
	di_app_archive = (dj_di_pointer)di_app_infusion_archive_data;

	// initialise serial port
	avr_serialInit(115200);

	core_init(mem, 2048);

	dj_exec_setRunlevel(RUNLEVEL_RUNNING);

}

void wukong_start()
{
	picokong_init((dj_di_pointer)di_app_infusion_archive_data);
}

void wukong_loop()
{
	wuobject_t *wuobject = NULL;
	// Process any incoming messages
	dj_hook_call(dj_core_pollingHook, NULL);
	if (dj_exec_getRunlevel() == RUNLEVEL_RUNNING) {
		// Propagate any dirty properties
		wkpf_propagate_dirty_properties();
		wkpf_get_next_wuobject_to_update(&wuobject);
	}
}
