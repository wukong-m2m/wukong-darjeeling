/*
 * execution.h
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
 

#ifndef __execution__
#define __execution__

#include "types.h"
#include "vmthread.h"
#include "vm.h"
#include "object.h"

#include "config.h"

// Must match numbers in Exception.java

#define ARITHMETIC_EXCEPTION 1
#define ARRAYINDEXOUTOFBOUNDS_EXCEPTION 2
#define ARRAYSTORE_EXCEPTION 3
#define CLASSCAST_EXCEPTION 4
#define CLASSUNLOADED_EXCEPTION 5
#define ILLEGALARGUMENT_EXCEPTION 6
#define ILLEGALTHREADSTATE_EXCEPTION 7
#define INDEXOUTOFBOUNDS_EXCEPTION 8
#define INFUSIONUNLOADDEPENDENCY_EXCEPTION 9
#define NATIVEMETHODNOTIMPLEMENTED_ERROR 10
#define NULLPOINTER_EXCEPTION 11
#define OUTOFMEMORY_ERROR 12
#define RUNTIME_EXCEPTION 13
#define STACKOVERFLOW_ERROR 14
#define STRINGINDEXOUTOFBOUNDS_EXCEPTION 15
#define VIRTUALMACHINE_ERROR 16


// Exported for RTC only
extern int16_t *intStack;
extern ref_t *refStack;
extern ref_t *localReferenceVariables;


// This is just here so we can pass both flags and global id in 1 4 byte parameter to callJavaMethod.
// Otherwise the 3 byte global id and 1 byte flags would take up 6 bytes since both get rounded up to
// 4 and 2 bytes when passed as parameters
typedef struct _dj_global_id_with_flags dj_global_id_with_flags;
struct _dj_global_id_with_flags
{
	dj_infusion *infusion;
	uint8_t entity_id;
	uint8_t flags;
};

// Original interface
void callMethod(dj_global_id methodImplId, bool virtualCall);
void callMethodFast(dj_global_id methodImplId, dj_di_pointer methodImpl, uint8_t flags, bool virtualCall);
// Optimised versions to use when the target impl is known at compile time
void callNativeMethod(dj_global_id methodImplId, dj_di_pointer methodImpl, bool virtualCall);
uint32_t callJavaMethod(dj_global_id_with_flags methodImplId, dj_di_pointer methodImpl, uint16_t frame_size);

void createThreadAndRunMethodToFinish(dj_global_id methodImplId);
bool dj_exec_use_rtc;
// End Exported for RTC only

extern dj_vm *currentVm;

void rtc_run_interpreter_if_not_aot_compiled();
void dj_exec_breakExecution();
void dj_exec_activate_thread(dj_thread *thread);
void dj_exec_deactivateThread(dj_thread *thread);

void dj_exec_throw(dj_object *obj, uint16_t throw_pc);
void dj_exec_throwHere(dj_object *obj);
void dj_exec_createAndThrow(int16_t exceptionType);

uint16_t dj_exec_getNumberOfObjectsOnReferenceStack();

void dj_exec_stackPushShort(int16_t value);
void dj_exec_stackPushInt(int32_t value);
void dj_exec_stackPushLong(int64_t value);
void dj_exec_stackPushRef(ref_t value);

int16_t dj_exec_stackPopShort();
int32_t dj_exec_stackPopInt();
int64_t dj_exec_stackPopLong();
ref_t dj_exec_stackPopRef();

int16_t dj_exec_stackPeekShort();
int32_t dj_exec_stackPeekInt();
ref_t dj_exec_stackPeekRef();
ref_t dj_exec_stackPeekDeepRef(int depth);

extern dj_thread *dj_currentThread;
#define dj_exec_getCurrentThread() dj_currentThread
#define dj_exec_setCurrentThread(thread) do { dj_currentThread = thread; } while (0)
dj_infusion *dj_exec_getCurrentInfusion();

void dj_exec_setVM(dj_vm *_vm);
#define dj_exec_getVM() (currentVm)

void dj_exec_updatePointers();

dj_frame *dj_exec_getCurrentFrame();
bool dj_exec_currentMethodIsRTCCompiled();

#ifdef DARJEELING_DEBUG_FRAME
void dj_exec_dumpFrame( dj_frame *frame );
void dj_exec_dumpFrameTrace( dj_frame *frame );
dj_frame *dj_exec_dumpExecutionState();	// Returns current frame
void dj_exec_debugCurrentFrame();
void dj_exec_debugFrameTrace();
#endif

#endif
