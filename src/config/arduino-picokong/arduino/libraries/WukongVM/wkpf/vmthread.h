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


dj_thread *dj_thread_create_and_run(dj_global_id methodImplId);
dj_thread *dj_thread_create();

void dj_thread_destroy(dj_thread *thread);

void dj_thread_pushFrame(dj_thread *thread, dj_frame *frame);
dj_frame *dj_thread_popFrame(dj_thread *thread);
char dj_thread_scanRootSetForRef(dj_thread *thread, ref_t ref);
void dj_thread_sleep(dj_thread *thread, dj_time_t time);
void dj_thread_wait(dj_thread * thread, dj_object * object, dj_time_t time);

void dj_thread_markRootSet(dj_thread *thread);
void dj_frame_updatePointers(dj_frame *frame);
void dj_thread_updatePointers(dj_thread *thread);

dj_frame *dj_frame_create(dj_global_id methodImpl);

dj_monitor_block * dj_monitor_block_create();
void dj_monitor_block_updatePointers(dj_monitor_block * monitor_block);
void dj_monitor_markRootSet(dj_monitor_block * monitor_block);

#define dj_frame_stackStartOffset(frame) ((char*)frame + sizeof(dj_frame))
#define dj_frame_stackEndOffset(frame) (dj_frame_stackStartOffset(frame) + (sizeof(int16_t) * dj_di_methodImplementation_getMaxStack(dj_global_id_getMethodImplementation(frame->method))))
#define dj_frame_stackLocalIntegerOffset(frame) (dj_frame_stackEndOffset(frame) + (sizeof(ref_t) * dj_di_methodImplementation_getReferenceLocalVariableCount(dj_global_id_getMethodImplementation(frame->method))))

#define dj_frame_getStackStart(frame) ((void*)dj_frame_stackStartOffset(frame))
#define dj_frame_getStackEnd(frame) ((void*)dj_frame_stackEndOffset(frame))

#define dj_frame_getReferenceStack(frame) ((ref_t*)(dj_frame_stackEndOffset(frame) - frame->nr_ref_stack * sizeof(ref_t)))
#define dj_frame_getIntegerStack(frame) ((int16_t*)(dj_frame_stackStartOffset(frame) + frame->nr_int_stack * sizeof(int16_t)))

#define dj_frame_getLocalReferenceVariables(frame) ((ref_t*)(dj_frame_stackEndOffset(frame)))
#define dj_frame_getLocalIntegerVariables(frame) ((int16_t*)(dj_frame_stackLocalIntegerOffset(frame)))

#define dj_frame_destroy(frame) dj_mem_free(frame)

#endif
