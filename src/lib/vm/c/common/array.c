/*
 * array.c
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
#include <stdlib.h>

#include "array.h"
#include "heap.h"
#include "debug.h"
#include "core.h"
#include "panic.h"

/**
 * Creates a new integer array.
 * @param type integer type, one of T_BOOLEAN, T_CHAR, T_BYTE, T_SHORT,	T_INT, used for type checking (instanceof etc)
 * @param size number of elements
 */
dj_int_array *dj_int_array_create(uint8_t type, uint16_t size)
{
	dj_int_array *arr;
	uint16_t byteSize = size;

	if ( type==T_BOOLEAN || type==T_BYTE || type==T_CHAR) { byteSize = size; } else
	if ( type==T_SHORT ) byteSize = size*sizeof(int16_t); else
	if ( type==T_INT ) byteSize = size*sizeof(int32_t); else
	if ( type==T_LONG ) byteSize = size*sizeof(int64_t); else
	{
        DEBUG_LOG(DBG_DARJEELING, "Unsupported array type: %d\n", type);
		dj_panic(DJ_PANIC_UNIMPLEMENTED_FEATURE);
	}

	// allocate array
	arr = (dj_int_array*)dj_mem_checked_alloc( byteSize + sizeof(dj_int_array), CHUNKID_INTARRAY );

    // let the caller deal with it
	if(arr == nullref) return NULL;

	// init array to zeroes
	memset(arr->data.bytes, 0, byteSize);

	// set array size
	arr->array.length = size;

	// set array type
	arr->type = type;

	// C be a harsh mistress!
	return arr;
}

/**
 * Destroys an integer array
 */
void dj_int_array_destroy(dj_int_array *array)
{
	dj_mem_free(array);
}

/**
 * Creates a new reference array.
 * @param runtime_class_id
 */
dj_ref_array *dj_ref_array_create(runtime_id_t runtime_class_id, uint16_t size)
{
	dj_ref_array *arr;

	// allocate array
	arr = (dj_ref_array*)dj_mem_checked_alloc( size * sizeof(ref_t) + sizeof(dj_ref_array), CHUNKID_REFARRAY );

    // let the caller deal with out of memory
	if(arr == nullref) return NULL;

	// set array size
	arr->array.length = size;

	// set array runtime class
	arr->runtime_class_id = runtime_class_id;

	// init array to zeroes
	memset(arr->refs, 0, size*sizeof(ref_t));

	// C be a harsh mistress!
	return arr;

}

/**
 * Destroys a reference array
 */
void dj_ref_array_destroy(dj_ref_array *array)
{
	dj_mem_free(array);
}

void dj_ref_array_updatePointers(dj_ref_array *array)
{
	int i;
	for (i=0; i<array->array.length; i++)
		array->refs[i] = dj_mem_getUpdatedReference(array->refs[i]);
}
