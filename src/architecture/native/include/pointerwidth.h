/*
 * pointerwidth.h
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
 
#ifndef __pointerwidth_h
#define __pointerwidth_h

#include <stdint.h>
#include <stddef.h>

#include "types.h"


// The null java reference
#define nullref ((ref_t)0)

typedef uint16_t ref_t;

extern char * ref_t_base_address;

// ref_t is now  only 16-bits wide for everyone. Thus,  we need a base
// address  to resolve  ref_t references  into 32-bits  pointers. This
// pointer is  actually declared in linux/main.c, and  assigned to the
// base address  of the heap *minus  a small quantity* to  allow us to
// distinguish  between null references  (ref_t ==  0) and  "the first
// object of the heap"

static inline void* REF_TO_VOIDP(ref_t ref) {return (ref != nullref ? (void*)((uint16_t)ref + ref_t_base_address) : NULL ) ;}
static inline ref_t VOIDP_TO_REF(void* ref) {return (ref != NULL ? (uint16_t)((char*)ref - ref_t_base_address) : nullref ) ;}

#define REF_TO_UINT32(ref) ((uint32_t)ref)
#define UINT32_TO_REF(ref) ((uint16_t)ref)


#endif // __pointerwidth_h
