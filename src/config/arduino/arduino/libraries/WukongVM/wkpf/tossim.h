/*
 * tossim.h
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
 
#ifndef __DARJEELING_TOSSIM_H__
#define __DARJEELING_TOSSIM_H__
#include "tosconfig.h"
#include "types.h"

// global variables for quick access
//static dj_thread *currentThread;
#define _global_execution_vm (getGlobalVariables()->vm)
// execution state
#define _global_execution_pc (getGlobalVariables()->pc)
#define _global_execution_code (getGlobalVariables()->code)

#define _global_execution_intStack (getGlobalVariables()->intStack)
#define _global_execution_refStack (getGlobalVariables()->refStack)

#define _global_execution_localReferenceVariables (getGlobalVariables()->localReferenceVariables)
#define _global_execution_localIntegerVariables (getGlobalVariables()->localIntegerVariables)

#define _global_execution_referenceParameters (getGlobalVariables()->referenceParameters)
#define _global_execution_integerParameters (getGlobalVariables()->integerParameters)

#define _global_execution_nrReferenceParameters (getGlobalVariables()->nrReferenceParameters)
#define _global_execution_nrIntegerParameters (getGlobalVariables()->nrIntegerParameters)

#define _global_execution_this (getGlobalVariables()->this)

#define _global_execution_nrOpcodesLeft (getGlobalVariables()->nrOpcodesLeft)
#ifdef DARJEELING_DEBUG
#define _global_execution_totalNrOpcodes (getGlobalVariables()->totalNrOpcodes)
#define _global_execution_oldPc (getGlobalVariables()->oldPc)
#endif
#ifdef DARJEELING_DEBUG_TRACE
#define _global_execution_callDepth (getGlobalVariables()->callDepth)
#endif


#define _global_radio_sendThreadId (getGlobalVariables()->sendThreadId)
#define _global_radio_receiveThreadId (getGlobalVariables()->receiveThreadId)


#define _global_heap_heap_base (getGlobalVariables()->heap_base)
#define _global_heap_panicExceptionObject (getGlobalVariables()->panicExceptionObject)
#define _global_heap_heap_size (getGlobalVariables()->heap_size)
#define _global_heap_refStack (getGlobalVariables()->heap_refStack)
#define _global_heap_refStackPointer (getGlobalVariables()->refStackPointer)

#define _global_heap_left_pointer (getGlobalVariables()->left_pointer)
#define _global_heap_right_pointer (getGlobalVariables()->right_pointer)

#ifdef DARJEELING_DEBUG_MEM_TRACE
#define _global_heap_nrTrace (getGlobalVariables()->nrTrace)
#endif




struct tossim_global_variables
{
	//from execution.c
	dj_vm *vm;

	// global variables for quick access
	//static dj_thread *currentThread;

	// execution state
	uint16_t pc;
	dj_di_pointer code;

	int16_t *intStack;
	ref_t *refStack;

	ref_t *localReferenceVariables;
	int16_t *localIntegerVariables;

	ref_t *referenceParameters;
	int16_t *integerParameters;

	uint8_t nrReferenceParameters;
	uint8_t nrIntegerParameters;

	ref_t this;

	int nrOpcodesLeft;
	#ifdef DARJEELING_DEBUG
	uint32_t totalNrOpcodes;
	uint16_t oldPc;
	#endif
	#ifdef DARJEELING_DEBUG_TRACE
	int callDepth = 0;
	#endif
	//------------------------------------------------
	//from javax_radio_radio.c
#ifdef WITH_RADIO
	int sendThreadId;
	int receiveThreadId;
#endif

	//------------------------------------------------
	//from heap.c
	char *heap_base;
	dj_object *panicExceptionObject;
	uint16_t heap_size;
	ref_t heap_refStack[HEAP_REFSTACKSIZE];
	uint8_t refStackPointer;

	void *left_pointer, *right_pointer;

	#ifdef DARJEELING_DEBUG_MEM_TRACE
	int nrTrace = 0;
	#endif



};

void* tossim_getDarjeelingGlobals();
void tossim_setDarjeelingGlobals(void *global_variables);
static inline struct tossim_global_variables* getGlobalVariables(){
	return tossim_getDarjeelingGlobals();
}

static inline void setGlobalVariables(struct tossim_global_variables* _global_variables){
	tossim_setDarjeelingGlobals((void *) _global_variables);
}

int tossim_printf(char *);
int tossim_debug(char *);
uint32_t tossim_getTime();

uint16_t tossim_getMaxPayloadLength();
int tossim_send(const char * message, int16_t receiverId, uint16_t length);
int tossim_wasAcked();

uint16_t tossim_peekMessageLength();
void * tossim_popMessageBuffer();
int tossim_getNrMessages();
#endif
