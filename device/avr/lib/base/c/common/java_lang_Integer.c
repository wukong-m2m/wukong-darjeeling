/*
 * java_lang_Integer.c
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
 
#include <string.h>
#include <stdio.h>

#include "heap.h"
#include "execution.h"
#include "panic.h"

#include "jlib_base.h"

// java.lang.String java.lang.Integer.toString(int)
void java_lang_Integer_java_lang_String_toString_int()
{
	char temp[8];
	char *str;
	int32_t value = dj_exec_stackPopInt();
	sprintf(temp,"%ld", (long)value);
	str = dj_mem_alloc(strlen(temp)+1, dj_vm_getSysLibClassRuntimeId(dj_exec_getVM(), BASE_CDEF_java_lang_String));

	if(str == NULL)
	{
    	dj_exec_createAndThrow(BASE_CDEF_java_lang_OutOfMemoryError);
    	return;
	}

	strcpy(str, temp);
	dj_exec_stackPushRef(VOIDP_TO_REF(str));
}
