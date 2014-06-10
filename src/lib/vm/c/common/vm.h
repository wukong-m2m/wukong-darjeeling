/*
 * vm.h
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
 
#ifndef __vm__
#define __vm__

#include "types.h"

void dj_vm_main(dj_di_pointer di_lib_archive, dj_di_pointer di_app_archive, dj_named_native_handler handlers[], uint8_t handlers_length);

dj_vm * dj_vm_create();
void dj_vm_destroy(dj_vm * vm);

void dj_vm_addInfusion(dj_vm * vm, dj_infusion * infusion);
int dj_vm_countInfusions(dj_vm * vm);
uint8_t dj_vm_getInfusionId(dj_vm * vm, dj_infusion * infusion);
dj_infusion * dj_vm_getInfusion(dj_vm * vm, int index);
dj_infusion * dj_vm_lookupInfusion(dj_vm * vm, dj_di_pointer name);
dj_infusion *dj_vm_loadSystemInfusion(dj_vm * vm, dj_di_pointer di);
bool dj_vm_safeToUnload(dj_vm *vm, dj_infusion * infusion);
void dj_vm_unloadInfusion(dj_vm *vm, dj_infusion * infusion);
dj_infusion * dj_vm_loadInfusion(dj_vm * vm, dj_di_pointer di, dj_named_native_handler native_handlers[], unsigned char numHandlers);

void dj_vm_setSystemInfusion(dj_vm *vm, dj_infusion * infusion);
dj_infusion * dj_vm_getSystemInfusion(dj_vm * vm);
void dj_vm_loadInfusionArchive(dj_vm * vm, dj_di_pointer archive_start, dj_named_native_handler native_handlers[], unsigned char numHandlers);

void dj_vm_runClassInitialisers(dj_vm * vm, dj_infusion * infusion);

void dj_vm_addThread(dj_vm * vm, dj_thread * thread);
int dj_vm_countThreads(dj_vm * vm);
int dj_vm_countLiveThreads(dj_vm * vm);
void dj_vm_checkFinishedThreads(dj_vm * vm);
void dj_vm_notify(dj_vm *vm, dj_object *object, bool all);
dj_thread *dj_vm_getThread(dj_vm * vm, int index);
dj_thread *dj_vm_getThreadById(dj_vm * vm, int id);
void dj_vm_removeThread(dj_vm * vm, dj_thread * thread);
char dj_vm_activateThread(dj_vm * vm, dj_thread * selectedThread);
dj_time_t dj_vm_getVMSleepTime(dj_vm * vm);

dj_monitor * dj_vm_getMonitor(dj_vm * vm, dj_object * object);
void dj_vm_removeMonitor(dj_vm * vm, dj_monitor * monitor);
void dj_monitor_updatePointers(dj_monitor * monitor);

void dj_vm_mem_markRootSet(dj_vm * vm);
void dj_vm_mem_updatePointers(dj_vm * vm);

char dj_vm_schedule(dj_vm * vm);

dj_di_pointer dj_vm_getRuntimeClassDefinition(dj_vm * vm, runtime_id_t id);
dj_global_id dj_vm_getRuntimeClass(dj_vm * vm, runtime_id_t id);

uint8_t dj_vm_getSysLibClassRuntimeId(dj_vm * vm, uint8_t entity_id);
dj_object * dj_vm_createSysLibObject(dj_vm * vm, uint8_t entity_id);

#endif
