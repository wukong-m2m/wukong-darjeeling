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
#include "wkcomm.h"
#include "config.h"
#include "avr.h"

extern const unsigned char di_lib_infusions_archive_data[];
extern const unsigned char di_app_infusion_archive_data[];

// From GENERATEDlibinit.c, which is generated during build based on the libraries in this config's libs.
extern dj_named_native_handler java_library_native_handlers[];
extern uint8_t java_library_native_handlers_length;


unsigned char mem[HEAPSIZE];

extern void ecocast_comm_handle_message(void *data);

int main()
{
	// Declared in djarchive.c so that the reprogramming code can find it.
	di_app_archive = (dj_di_pointer)di_app_infusion_archive_data;

	// initialise serial port
	avr_serialInit(115200);

	core_init(mem, HEAPSIZE);

	// uint8_t msg_payload[7] = { 0, 0, 1, 138, 226, 8, 149 };

	// uint8_t msg_payload[3+24] = { 0, 0, 1,      231,224,241,224,128,129,132,96,128,131,232,224,241,224,128,129,139,127,128,131,138,226,8,149 };

	// uint8_t msg_payload[3+30] = { 0, 0, 4,   	231,224,241,224,128,129,132,96,128,131,232,224,241,224,128,129,139,127,128,131,104,231,118,229,132,227,146,225,8,149 };

	//	uint8_t msg_payload[3+26] = { 0, 0, 2,   	231,224,241,224,128,129,132,96,128,131,232,224,241,224,128,129,139,127,128,131,132,227,146,225,8,149 };

	// wkcomm_received_msg msg;
	// msg.src = 0;
	// msg.seqnr = 0;
	// msg.command = 0x20; // ECOCAST_COMM_ECOCOMMAND
	// msg.payload = msg_payload;
	// msg.length = 3+26;

	// ecocast_comm_handle_message((void *)&msg);

	// DARJEELING_PRINTF("RETURN VALUE: %x %x %x %x\n", *((uint8_t *)msg_payload+0), *((uint8_t *)msg_payload+1), *((uint8_t *)msg_payload+2), *((uint8_t *)msg_payload+3));

	// Listen to the radio
	while(true)
		dj_hook_call(dj_core_pollingHook, NULL);

	return 0;
}



