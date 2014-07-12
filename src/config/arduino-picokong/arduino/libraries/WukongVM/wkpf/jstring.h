/*
 * string.h
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

#ifndef __jstring__
#define __jstring__

#include "config.h"
#include "types.h"
#include "vm.h"
#include "global_id.h"
#include "heap.h"
#include "parse_infusion.h"


dj_object * dj_jstring_createFromStr(dj_vm *vm, char * str);
dj_object * dj_jstring_createFromGlobalId(dj_vm *vm, dj_global_id stringId);

#endif
