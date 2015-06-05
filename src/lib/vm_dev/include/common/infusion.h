/*
 * infusion.h
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
 
#ifndef __infusion__
#define __infusion__

#include "types.h"
#include "parse_infusion.h"
#include "debug.h"

#include "config.h"

dj_infusion *dj_infusion_create(dj_di_pointer staticFieldInfo, int nr_referenced_infusions);
void dj_infusion_destroy(dj_infusion *infusion);
void dj_infusion_markRootSet(dj_infusion *infusion);
void dj_infusion_updatePointers(dj_infusion *infusion);
int dj_infusion_getReferencedInfusionIndex(dj_infusion *infusion, dj_infusion *searchInfusion);
void dj_infusion_getName(dj_infusion * infusion, char * str, int strLength);

dj_di_pointer dj_infusion_getMethodImplementation(dj_infusion *infusion, int entity_id);
dj_di_pointer dj_infusion_getClassDefinition(dj_infusion * infusion, int entity_id);
dj_di_pointer dj_infusion_getString(dj_infusion * infusion, int entity_id);

// functions that need to be quick and should be inlined
static inline dj_infusion * dj_infusion_resolve(dj_infusion *infusion, int id)
{
    dj_infusion *result;
	result=(id==0)?infusion:infusion->referencedInfusions[id-1];
    return result;
}

#endif
