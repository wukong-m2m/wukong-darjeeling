/*
 * infusion.c
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
 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "infusion.h"
#include "heap.h"
#include "execution.h"
#include "panic.h"

//platform-specific header
#include "config.h"


/**
 * Creates a new infusion.
 * @param staticFieldInfo a pointer to a static field info record
 * @param nrImportedInfusions the number of imported infusions
 * @return a new dj_infusion struct, allocated on the VM heap
 */
dj_infusion *dj_infusion_create(dj_di_pointer staticFieldInfo, int nrImportedInfusions)
{
	dj_infusion *ret;
	int staticFieldsSize;
	void *staticFields;

	// allocate memory for the static fields.
	// We put all the fields in one memory chunk,
	// so here we calculate the total size we need
	staticFieldsSize = dj_di_staticFieldInfo_getNrRefs(staticFieldInfo) * sizeof(ref_t);
	staticFieldsSize += dj_di_staticFieldInfo_getNrBytes(staticFieldInfo) * sizeof(uint8_t);
	staticFieldsSize += dj_di_staticFieldInfo_getNrShorts(staticFieldInfo) * sizeof(uint16_t);
	staticFieldsSize += dj_di_staticFieldInfo_getNrInts(staticFieldInfo) * sizeof(uint32_t);
	staticFieldsSize += dj_di_staticFieldInfo_getNrLongs(staticFieldInfo) * sizeof(uint64_t);

	// allocate infusion struct, plus the memory needed for the static fields in
	// one block to save on heap complexity (less chunks on the heap)
	ret = (dj_infusion*)dj_mem_alloc(sizeof(dj_infusion) + staticFieldsSize + nrImportedInfusions*sizeof(dj_infusion*), CHUNKID_INFUSION);

    if(ret == NULL)
    	return NULL;

	// bookkeeping for the gc
	ret->nr_static_refs = dj_di_staticFieldInfo_getNrRefs(staticFieldInfo);

	// initialise to zero
    // GS-09/10/2008-16:06(AEST) TODO: please, could we have named constants here ?
	ret->classList = 0;
	ret->methodImplementationList = 0;
	ret->next = 0;
	ret->native_handler = 0;

	// initialise to DJ_DI_NOT_SET
	ret->header = DJ_DI_NOT_SET;
	ret->classList = DJ_DI_NOT_SET;
	ret->methodImplementationList = DJ_DI_NOT_SET;
	ret->stringTable = DJ_DI_NOT_SET;

	// allocate memory for the static fields and set the
	// pointers
	staticFields = (void*)((size_t)ret + sizeof(dj_infusion));
	ret->staticReferenceFields = (ref_t*)staticFields;
	staticFields += dj_di_staticFieldInfo_getNrRefs(staticFieldInfo) * sizeof(ref_t);
	ret->staticByteFields = (uint8_t*)staticFields;
	staticFields += dj_di_staticFieldInfo_getNrBytes(staticFieldInfo) * sizeof(uint8_t);
	ret->staticShortFields = (uint16_t*)staticFields;
	staticFields += dj_di_staticFieldInfo_getNrShorts(staticFieldInfo) * sizeof(uint16_t);
	ret->staticIntFields = (uint32_t*)staticFields;
	staticFields += dj_di_staticFieldInfo_getNrInts(staticFieldInfo) * sizeof(uint32_t);
	ret->staticLongFields = (uint64_t*)staticFields;

	// init static fields to 0
	memset((void*)((size_t)ret + sizeof(dj_infusion)), 0, staticFieldsSize);

	// set the referenced infusions pointer
	ret->nr_referenced_infusions = nrImportedInfusions;
	ret->referencedInfusions = (dj_infusion**)((size_t)ret + sizeof(dj_infusion) + staticFieldsSize);

	return ret;
}

/**
 * Destroys an infusion and frees it from the VM heap.
 */
void dj_infusion_destroy(dj_infusion *infusion)
{
	// free the infusion struct
	dj_mem_free(infusion);
}

/**
 * When infusion A imports infusion B, this method gives the import index of B in A.
 * @param importer the infusion that imports (A)
 * @param importee the infusion that is imported (B)
 * @return the import index of B in A, or -1 if A does not import B. By specification, if A==B, this method returns 0.
 */
int dj_infusion_getReferencedInfusionIndex(dj_infusion *importer, dj_infusion *importee)
{
	int i;

	// by specification (infusion_id==0 means a reference back to itself)
	if (importer==importee)
		return 0;

	// search the list of imported infusions
	for (i=0; i<importer->nr_referenced_infusions; i++)
		if (importee==importer->referencedInfusions[i])
			return i+1;

	// not found, return -1
	return -1;
}

void dj_infusion_markRootSet(dj_infusion *infusion)
{
	int i;

	dj_mem_setChunkColor(infusion, TCM_BLACK);

	for (i=0; i<infusion->nr_static_refs; i++)
		dj_mem_setRefColor(infusion->staticReferenceFields[i], TCM_GRAY);
}

void dj_infusion_updatePointers(dj_infusion *infusion)
{
	int i;
	int shift;

	// update static references
	for (i=0; i<infusion->nr_static_refs; i++)
		infusion->staticReferenceFields[i] = dj_mem_getUpdatedReference(infusion->staticReferenceFields[i]);

	// update pointers to other infusions in the import table
	for (i=0; i<infusion->nr_referenced_infusions; i++)
		infusion->referencedInfusions[i] = dj_mem_getUpdatedPointer(infusion->referencedInfusions[i]);

	// next pointer
	infusion->next = dj_mem_getUpdatedPointer(infusion->next);

	// update member pointers
	// TODO fix this crap
	shift = dj_mem_getChunkShift(infusion);
	infusion->staticByteFields = (uint8_t*)((void*)infusion->staticByteFields - shift);
	infusion->staticShortFields = (uint16_t*)((void*)infusion->staticShortFields - shift);
	infusion->staticIntFields = (uint32_t*)((void*)infusion->staticIntFields - shift);
	infusion->staticLongFields = (uint64_t*)((void*)infusion->staticLongFields - shift);
	infusion->staticReferenceFields = (ref_t*)((void*)infusion->staticReferenceFields - shift);
	infusion->referencedInfusions = (dj_infusion**)((void*)infusion->referencedInfusions - shift);
}

void dj_infusion_getName(dj_infusion * infusion, char * str, int strLength)
{
	int i;
	uint8_t ch;

	dj_di_pointer name = dj_di_header_getInfusionName(infusion->header);

	for (i=0; i<strLength; i++)
	{
		str[i] = ch = (char)dj_di_getU8(name);
		if (ch==0) break;
		name ++;
	}

}

dj_di_pointer dj_infusion_getClassDefinition(dj_infusion * infusion, int entity_id)
{
	return dj_di_parentElement_getChild(infusion->classList, entity_id);
}

dj_di_pointer dj_infusion_getMethodImplementation(dj_infusion *infusion, int entity_id)
{
	return dj_di_parentElement_getChild(infusion->methodImplementationList, entity_id);
}

dj_di_pointer dj_infusion_getString(dj_infusion * infusion, int entity_id)
{
	return dj_di_stringtable_getElementBytes(infusion->stringTable, entity_id);
}

