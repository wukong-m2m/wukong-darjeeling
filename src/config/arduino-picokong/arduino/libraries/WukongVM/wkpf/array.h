/*
 * array.h
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
 
#ifndef __array__
#define __array__

#include "config.h"
#include "types.h"

/**
 * Array types, these are taken straight from the Java specification. Unsupported types float, double and long
 * are included for completeness.
 */
enum ArrayType
{
	T_BOOLEAN = 4,
	T_CHAR = 5,
	T_FLOAT = 6,
	T_DOUBLE = 7,
	T_BYTE = 8,
	T_SHORT = 9,
	T_INT = 10,
	T_LONG = 11
};


typedef struct _dj_array dj_array;
typedef struct _dj_int_array dj_int_array;
typedef struct _dj_ref_array dj_ref_array;

struct _dj_array
{
	uint16_t length;
}
#ifdef PACK_STRUCTS
__attribute__ ((__packed__))
#endif
;

struct _dj_int_array
{
	dj_array array;
	uint8_t type;
	union
	{
		int8_t bytes[0];
		int16_t shorts[0];
		int32_t ints[0];
		int64_t longs[0];
	} data;
}
#ifdef PACK_STRUCTS
__attribute__ ((__packed__))
#endif
;

struct _dj_ref_array
{
	dj_array array;
	runtime_id_t runtime_class_id;
	ref_t refs[0];
}
#ifdef PACK_STRUCTS
__attribute__ ((__packed__))
#endif
;

dj_int_array *dj_int_array_create(uint8_t type, uint16_t size);
void dj_int_array_destroy(dj_int_array *array);

dj_ref_array *dj_ref_array_create(runtime_id_t runtime_class_id, uint16_t size);
void dj_ref_array_destroy(dj_ref_array *array);
void dj_ref_array_updatePointers(dj_ref_array *array);

#endif
