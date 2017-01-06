/*
 * javax_darjeeling_Darjeeling.c
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
#include <string.h>
#include <stdlib.h>

// generated at infusion time
#include "jlib_base.h"

#include "vm.h"
#include "execution.h"
#include "heap.h"
#include "djtimer.h"
#include "panic.h"
#include "array.h"

int32_t nesc_getNodeId();
// void javax.darjeeling.Darjeeling.assertTrue(int, boolean)
void javax_darjeeling_Darjeeling_void_assertTrue_int_boolean()
{
	// pop argument from the stack
	int32_t value = dj_exec_stackPopShort();
	int32_t id = dj_exec_stackPopInt();
	if (value==0)
		DARJEELING_PRINTF("%c[31mASSERT[%3d] FAILED%c[0m\n", 0x1b, (int)id, 0x1b);
	else
		DARJEELING_PRINTF("%c[32mASSERT[%3d] PASSED%c[0m\n", 0x1b, (int)id, 0x1b);
}

void javax_darjeeling_Darjeeling_void__print_java_lang_String()
{
	int i;

	// Pop string object from the stack.
	BASE_STRUCT_java_lang_String * stringObject = (BASE_STRUCT_java_lang_String*)REF_TO_VOIDP(dj_exec_stackPopRef());

	// Get byte array
	dj_int_array * byteArray = (dj_int_array*)REF_TO_VOIDP(stringObject->value);

	for (i=0; i<byteArray->array.length; i++)
		DARJEELING_PRINTF("%c", byteArray->data.bytes[i]);
}


// int javax.darjeeling.Darjeeling.getNodeId()
void javax_darjeeling_Darjeeling_int_getNodeId()
{
	dj_exec_stackPushInt(nesc_getNodeId());
}

//int javax.darjeeling.Darjeeling.getMemFree()
void javax_darjeeling_Darjeeling_int_getMemFree()
{
	dj_exec_stackPushInt(dj_mem_getFree());
}

//int javax.darjeeling.Darjeeling.random()
void javax_darjeeling_Darjeeling_int_random()
{
	dj_exec_stackPushInt(rand());
}

//short javax.darjeeling.Darjeeling.getNrThreads()
void javax_darjeeling_Darjeeling_short_getNrThreads()
{
	dj_exec_stackPushShort(dj_vm_countThreads(dj_exec_getVM()));
}

//Thread javax_darjeeling_Darjeeling.getThread(short)
void javax_darjeeling_Darjeeling_java_lang_Thread_getThread_short()
{
	dj_thread *thread;
        int index = dj_exec_stackPopShort();

        // check for out of bounds
        if ( (index<0) || (index>=dj_vm_countThreads(dj_exec_getVM())) )
                dj_exec_throwHere(dj_vm_createSysLibObject(dj_exec_getVM(), BASE_CDEF_java_lang_IndexOutOfBoundsException));
        else
        {
                thread = dj_vm_getThread(dj_exec_getVM(), index);
                dj_exec_stackPushRef(VOIDP_TO_REF(thread));
        }
}
