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
#include "hooks.h"
#include "core.h"
#include "djarchive.h"
#include "wkpf_main.h"

#include "avr.h"


unsigned char mem[HEAPSIZE];

int main()
{
	// initialise serial port
	avr_serialInit(115200);

	core_init(mem, HEAPSIZE);
	dj_exec_setRunlevel(RUNLEVEL_RUNNING);
	wkpf_picokong((dj_di_pointer)di_app_infusion_archive_data);

	// Listen to the radio
	while(true)
		dj_hook_call(dj_core_pollingHook, NULL);

	return 0;
}
