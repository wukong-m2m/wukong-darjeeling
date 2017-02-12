/*
 * vmthread.h
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
 
#ifndef __vmthread_h
#define __vmthread_h

#include "parse_infusion.h"
#include "object.h"

#include "config.h"


enum _dj_threadstatus
{
	THREADSTATUS_CREATED = 0,
	THREADSTATUS_BLOCKED = 1,
	THREADSTATUS_RUNNING = 2,
	THREADSTATUS_SLEEPING = 3,
	THREADSTATUS_BLOCKED_FOR_MONITOR = 4,
	THREADSTATUS_WAITING_FOR_MONITOR = 5,
	THREADSTATUS_BLOCKED_FOR_IO = 6,
	THREADSTATUS_FINISHED = 7,
	THREADSTATUS_UNHANDLED_EXCEPTION = 8
};


dj_thread *dj_thread_create();

void dj_thread_destroy(dj_thread *thread);

void dj_thread_pushFrame(dj_frame *frame);
dj_frame *dj_thread_popFrame();
char dj_thread_scanRootSetForRef(dj_thread *thread, ref_t ref);
void dj_thread_sleep(dj_thread *thread, dj_time_t time);
void dj_thread_wait(dj_thread * thread, dj_object * object, dj_time_t time);

void dj_thread_markRootSet(dj_thread *thread);
void dj_frame_updatePointers(dj_frame *frame);
void dj_thread_updatePointers(dj_thread *thread);

dj_frame *dj_frame_create_fast(dj_global_id methodImplId, dj_di_pointer methodImpl);

dj_monitor_block * dj_monitor_block_create();
void dj_monitor_block_updatePointers(dj_monitor_block * monitor_block);
void dj_monitor_markRootSet(dj_monitor_block * monitor_block);

#define dj_frame_stackStartOffset(methodImpl)                    (0)
#define dj_frame_stackEndOffset(methodImpl)                      (sizeof(int16_t) * dj_di_methodImplementation_getMaxStack(methodImpl))
#define dj_frame_getStackStart(frame, methodImpl)				 ((void*)((uint16_t)frame + sizeof(dj_frame) + (uint16_t)dj_frame_stackStartOffset(methodImpl)))
#define dj_frame_getStackEnd(frame, methodImpl)                  ((void*)((uint16_t)frame + sizeof(dj_frame) + (uint16_t)dj_frame_stackEndOffset(methodImpl)))

#define dj_frame_getReferenceStackBase(frame, methodImpl)        ((ref_t*)(dj_frame_getStackStart(frame, methodImpl)))
#define dj_frame_getIntegerStackBase(frame, methodImpl)          ((int16_t*)(dj_frame_getStackEnd(frame, methodImpl) - sizeof(int16_t)))

#define dj_frame_getLocalReferenceVariables(frame, methodImpl)   ((ref_t*)((char*)frame + sizeof(dj_frame) + dj_frame_stackEndOffset(methodImpl)))
// (note the header now assumes 2 byte pointers, so VMs on larger architectures will need to do some extra work!!!!)
#define dj_frame_getLocalIntegerVariables(frame, methodImpl)     ((int16_t*)((char*)frame + sizeof(dj_frame) + dj_di_methodImplementation_getOffsetToLocalIntegerVariables((uint16_t)methodImpl)))

#ifndef EXECUTION_FRAME_ON_STACK
#define dj_frame_destroy(frame) dj_mem_free(frame)
#endif

#endif // __vmthread_h
