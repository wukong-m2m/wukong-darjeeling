/*
 * types.h
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
 
#ifndef __types_h
#define __types_h

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

// platform-specific header files
#include "program_mem.h"
#include "pointerwidth.h"


// the "ref_t"  type and  the "null" constant  are now defined  in the
// platform-specific file "pointerwidth.h", but in both cases they are
// 16-bits wide.

// common types
typedef uint16_t runtime_id_t;
typedef long long int dj_time_t;

// keep this a multiple of 2, to keep the size of the blocks a multiple of 2 (for 16-bit architectures)
#define MONITOR_BLOCK_SIZE 8

typedef struct _dj_local_id dj_local_id;
typedef struct _dj_global_id dj_global_id;

typedef struct _dj_object dj_object;

typedef struct _dj_thread dj_thread;
typedef struct _dj_frame dj_frame;
typedef struct _dj_monitor dj_monitor;
typedef struct _dj_monitor_block dj_monitor_block;

typedef struct _dj_infusion dj_infusion;
typedef struct _dj_vm dj_vm;
typedef struct _dj_named_native_handler dj_named_native_handler;

#ifdef AOT_SAFETY_CHECKS
typedef struct _rtc_safety_method_signature rtc_safety_method_signature;
struct  _rtc_safety_method_signature
{
	uint8_t nr_int_args;
	uint8_t nr_ref_args;
	uint8_t return_type;
};
#endif // AOT_SAFETY_CHECKS


/**
 * A two-byte typle that references entities. The infusion_id points to an infusion and the entity_id indexes
 * a certain entity within that infusion. The local ID is called 'local' because it only makes sense within the context of
 * the infusion in which it is found. This is because the infusion_id indexes into an import list in the infusion. Local IDs
 * can be resolved into global IDs.
 */
struct _dj_local_id
{
	/** Infusion ID */
	uint8_t infusion_id;

	/** Entity ID */
	uint8_t entity_id;
}
#ifdef PACK_STRUCTS
__attribute__ ((__packed__))
#endif
;

/**
 * A pointer/byte tuple that holds a pointer to an infusion and a byte that indexes a certain entity within
 * that infusion.
 */
struct _dj_global_id
{
	dj_infusion *infusion;
	uint8_t entity_id;
}
#ifdef PACK_STRUCTS
__attribute__ ((__packed__))
#endif
;

typedef void (*native_method_function_t)(void);

struct _dj_named_native_handler
{
	char * name;
	const DJ_PROGMEM native_method_function_t * handlers;
};

struct _dj_object
{
	// nothing here, the Object struct is more syntacting than functional
}
#ifdef PACK_STRUCTS
__attribute__ ((__packed__))
#endif
;

struct _dj_frame
{
	dj_frame * parent;							// stack implemented as a linked list
//	dj_infusion * infusion;						// for resolving references
//	dj_di_pointer method;						// method that is executing in this frame
	dj_global_id method;

#ifdef EXECUTION_DISABLEINTERPRETER_COMPLETELY
	uint16_t pc;								// program counter, return adress
#endif
	int16_t* saved_intStack;					// the saved value of intStack, replaces nr_int_stack because for RTC methods, intStack doesn't point to the stack frame, and thus can't be calculated from nr_int_stack
	ref_t* saved_refStack;						// same for refStack, just to be consistent (it's still in the stack frame, so actually we could still calculate it, for RTC methods as well)
}
#ifdef PACK_STRUCTS
__attribute__ ((__packed__))
#endif
;

struct _dj_thread
{
	dj_time_t scheduleTime;
	int16_t id;								// unique thread id
	uint8_t status;
	uint8_t priority;

	dj_frame * frameStack;

	// runnable object
	dj_object * runnable;

	// monitor object in case of blocked
	dj_object * monitorObject;

	// threads are stored as a linked list
	dj_thread * next;

}
#ifdef PACK_STRUCTS
__attribute__ ((__packed__))
#endif
;

struct _dj_monitor
{
	dj_object * object;
	dj_thread * owner;
	uint8_t count;
	uint8_t waiting_threads;
}
#ifdef PACK_STRUCTS
__attribute__ ((__packed__))
#endif
;

struct _dj_monitor_block
{
	dj_monitor_block *next;
	dj_monitor monitors[MONITOR_BLOCK_SIZE];
	uint8_t count;
#ifdef ALIGN_16
	uint8_t PADDING;
#endif
}
#ifdef PACK_STRUCTS
__attribute__ ((__packed__))
#endif
;

struct _dj_infusion
{

	// DI file elements
	dj_di_pointer header;
	dj_di_pointer classList;
	dj_di_pointer methodImplementationList;
	dj_di_pointer stringTable;
	dj_di_pointer methodImplementationCodeList;

	// pointer to native method handler
	const DJ_PROGMEM native_method_function_t * native_handlers;

	// for dynamic adress translation
	runtime_id_t class_base;

	// gc bookkeeping
	uint8_t nr_static_refs;

	// nr of referenced infusions
	uint8_t nr_referenced_infusions;

	// Infusions are stored as a linked list
	dj_infusion * next;

	// static fields
	ref_t * staticReferenceFields;
	uint8_t * staticByteFields;
	uint16_t * staticShortFields;
	uint32_t * staticIntFields;
	uint64_t * staticLongFields;

	// infusion mapping
	dj_infusion ** referencedInfusions;
#ifdef ALIGN_16
	uint8_t PADDING;
#endif

}
#ifdef PACK_STRUCTS
__attribute__ ((__packed__))
#endif
;

struct _dj_vm
{
	dj_infusion *infusions;

	// 20130322 Niels Reijers: not sure if this is the best place to store
	//                         this but it's better than a global variable.
	dj_di_pointer di_app_infusion_archive_data;

	// dj_thread *currentThread;

	uint16_t threadNr;
	uint16_t numMonitors;

	dj_infusion *systemInfusion;

	dj_thread *threads;
	dj_monitor_block *monitors;

}
#ifdef PACK_STRUCTS
__attribute__ ((__packed__))
#endif
;

#endif
