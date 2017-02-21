/*
 * object.c
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

#include "object.h"
#include "heap.h"
#include "execution.h"
#include "panic.h"

/**
 * Constructs a new object.
 * @param nr_refs the number of references to allocate
 * @param non_ref_size the number of bytes to allocate for integer values
 * @return a new object instance
 */
dj_object *dj_object_create(runtime_id_t type, int nr_refs, int non_ref_size)
{
	dj_object *ret;

#ifdef ALIGN_16
	if (non_ref_size&1) non_ref_size++;
#endif

	uint16_t size = nr_refs * sizeof(ref_t) + non_ref_size;

	ret = (dj_object*)dj_mem_checked_alloc(size + sizeof(dj_object), type);

	if(ret == NULL) return NULL;

	// init fields to 0
	memset((void*)ret, 0, size);

	return ret;
}

runtime_id_t dj_object_getRuntimeId(dj_object * object)
{
	return dj_mem_getChunkId(object);
}

ref_t * dj_object_getReferences(dj_object * object)
{
	uint16_t refOffset = dj_di_classDefinition_getOffsetOfFirstReference(dj_vm_getRuntimeClassDefinition(dj_exec_getVM(), dj_mem_getChunkId(object)));

#ifdef ALIGN_16
	if (refOffset&1) refOffset++;
#endif
	
	return (ref_t*)((size_t)object + refOffset);
}


