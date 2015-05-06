/*
 * object.h
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
 
#ifndef __object__
#define __object__

#include "config.h"
#include "types.h"
#include "vm.h"
#include "global_id.h"
#include "heap.h"
#include "parse_infusion.h"

dj_object *dj_object_create(runtime_id_t type, int nr_refs, int non_ref_size);
runtime_id_t dj_object_getRuntimeId(dj_object * object);

dj_vm * dj_exec_getVM();

ref_t * dj_object_getReferences(dj_object * object);

#endif
