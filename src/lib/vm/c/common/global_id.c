/*
 * global_id.c
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
 
#include "global_id.h"
#include "infusion.h"
#include "array.h"
#include "heap.h"
#include "vm.h"
#include "execution.h"
#include "object.h"
#include "debug.h"
#include "parse_infusion.h"

#include "jlib_base.h"

/**
 * Resolves the infusion part of a local id, resulting in a global id.
 * @param infusion infusion to which the local id belongs
 * @param local_id the local id that needs resolving
 * @return a global id
 */
dj_global_id dj_global_id_resolve(dj_infusion *infusion, dj_local_id local_id)
{
	dj_global_id ret;
	//DEBUG_ENTER_NEST_LOG(DBG_DARJEELING, "dj_global_id_resolve(%p,{%d,%d})\n",infusion,local_id.infusion_id,local_id.entity_id);
	ret.infusion = dj_infusion_resolve(infusion, local_id.infusion_id);
	ret.entity_id = local_id.entity_id;

	//DEBUG_EXIT_NEST_LOG(DBG_DARJEELING, "dj_global_id_resolve(): {%p,%d}\n",ret.infusion,ret.entity_id);
	return ret;
}

/**
 * Maps a local id into a local id.
 * @param global_id global id to be mapped
 * @param infusion infusion to map the global id to
 * @return a local id, local to the given infusion
 */
dj_local_id dj_global_id_mapToInfusion(dj_global_id gobal_id, dj_infusion *infusion)
{
	dj_local_id ret;
	ret.infusion_id = dj_infusion_getReferencedInfusionIndex(infusion, gobal_id.infusion);
	ret.entity_id = gobal_id.entity_id;
	return ret;
}

/**
 * Gets the parent of a class.
 * @param global_class_id a global id pointing to a class entity
 * @return a global id pointing to the parent of the class
 */
static inline dj_global_id dj_global_id_getParent(dj_global_id global_class_id)
{
	dj_di_pointer class_def = (dj_di_pointer)dj_global_id_getClassDefinition(global_class_id);
	dj_local_id parent_id = dj_di_classDefinition_getSuperClass(class_def);
	dj_global_id parent = dj_global_id_resolve(global_class_id.infusion, parent_id);

	return parent;
}

/**
 * Tests for equality on global ids.
 * @param a first global id to compare
 * @param b second global id to compare
 * @return true if a and b are equal, false otherwise
 */
char dj_global_id_equals(dj_global_id a, dj_global_id b)
{
	return ( (a.infusion==b.infusion) && (a.entity_id==b.entity_id) );
}

/**
 * Helper function for dj_global_id_implements.
 *
 * Checks whether 'class' itself  (but not its superclasses) implements
 * 'interface'  or any  of  its superinterfaces.
 */
static inline char dj_global_id_implementsDirectly(dj_global_id class, dj_global_id interface)
{
	int i;
	dj_di_pointer classDef = dj_global_id_getClassDefinition(class);
	dj_global_id implementedInterface;

	// iterate over the interface list
	for (i=0; i<dj_di_classDefinition_getNrInterfaces(classDef); i++)
	{
		implementedInterface = dj_global_id_resolve(
				class.infusion,
				dj_di_classDefinition_getInterface(classDef, i)
				);

		if (dj_global_id_equals(implementedInterface, interface))
			return 1;

	}

	// not found, return false
	return 0;
}

/**
 * Tests if a class implements an interface. used in the INSTANCEOF and CHECKCAST opcodes.
 * @param class a global id pointing to a class
 * @param interface a global id pointing to an interface
 * @return true is the class implements the interface, false otherwise
 */
char dj_global_id_implements(dj_global_id class, dj_global_id interface)
{
	dj_global_id finger = class;

	// java.lang.Object doesn't implement any interface and has no parent
	while (!dj_global_id_isJavaLangObject(finger))
	{
		if (dj_global_id_implementsDirectly(finger, interface))
			return 1;

		finger = dj_global_id_getParent(finger);
	}

	return 0;

}

/**
 * Tests if a class is equal to, or a subclass of another class. used in the INSTANCEOF and CHECKCAST opcodes.
 * @param child a global id pointing to a class
 * @param parent a global id pointing to a class
 * @return true if child==parent, or if child extends parent. False otherwise
 */
char dj_global_id_isEqualToOrChildOf(dj_global_id child, dj_global_id parent)
{
	dj_global_id finger;
    DEBUG_ENTER_NEST_LOG(DBG_DARJEELING, "isEqualToOrChildOf({%p,%d},{%p,%d})\n",
                         child.infusion,child.entity_id,
                         parent.infusion,parent.entity_id);

    // if equal return true
    if (dj_global_id_isJavaLangObject(parent))
	{
		DEBUG_EXIT_NEST_LOG(DBG_DARJEELING, "Parent is Object\n");
		return 1;
	}


    if (dj_global_id_equals(child, parent))
    {
        DEBUG_EXIT_NEST_LOG(DBG_DARJEELING, "True:child equals parent\n");
		return 1;
    }

    // if equal return true
	if (dj_global_id_isJavaLangObject(child))
    {
        DEBUG_EXIT_NEST_LOG(DBG_DARJEELING, "object is Object\n");
		return 0;
    }

	// check child is a subclass of parent
	finger = dj_global_id_getParent(child);
    DEBUG_LOG(DBG_DARJEELING, "init finger on {%p,%d}\n",finger.infusion,finger.entity_id);

	while (!dj_global_id_isJavaLangObject(finger))
	{
        DEBUG_LOG(DBG_DARJEELING, "finger={%p,%d}\n",finger.infusion,finger.entity_id);

		// if the test class and the parent class are equal, return 1
		if (dj_global_id_equals(finger, parent))
        {
            DEBUG_EXIT_NEST_LOG(DBG_DARJEELING, "True:finger==parent\n");
			return 1;
        }

		finger = dj_global_id_getParent(finger);
	}
	DEBUG_EXIT_NEST_LOG(DBG_DARJEELING, "False:parent {%p,%d} not found among ancestors of {%p,%d}\n",
                        parent.infusion,parent.entity_id,
                        child.infusion,child.entity_id);
	return 0;

}

/**
 * Tests wether the class <pre>refClass</pre> is of the type <pre>testType</pre>.
 * This method tests if refClass implements testType (if testType is an interface)
 * and whether refClass implements testType (if testType is a class)
 * @param refClass class to test
 * @param testType class to test against.
 * @return true if refClass is of type testType, false otherwise.
 */
char dj_global_id_testClassType(dj_global_id refClass, dj_global_id testType)
{
	// TODO only check for implements if testType is an interface,
	// only check for extends if testType is a class
    DEBUG_ENTER_NEST_LOG(DBG_DARJEELING, "dj_global_id_testClassType({%p,%d},{%p,%d})\n",
                         refClass.infusion,refClass.entity_id,
                         testType.infusion,testType.entity_id);

	// check if refClass is equal to, or subclass of testType
	if (dj_global_id_isEqualToOrChildOf(refClass, testType))
    {
        DEBUG_EXIT_NEST(DBG_DARJEELING, "true");
		return 1;
    }

	// check if refClass implements testType as an interface
	if (dj_global_id_implements(refClass, testType))
    {
        DEBUG_EXIT_NEST(DBG_DARJEELING, "true");
		return 1;
    }

    DEBUG_EXIT_NEST(DBG_DARJEELING, "false");
	return 0;
}

/**
 * Checks if the object on the heap (wich is a array of numeric elements) is of the
 * type (local_infusion_id, local_class_id). Note that Infuser translates the type
 * 'array of numeric elements' into (local_infusion_id=INTARRAY_INFUSION_ID,
 * local_class_id=type).
 *
 */
static inline char dj_global_id_checkIntArrayType(void *ref, dj_local_id typeId)
{
	// make sure that the type we are checking against is an array of numeric elements.
	// if not, return 0 (false)
	if (typeId.infusion_id!=INTARRAY_INFUSION_ID)
		return 0;

	// get the array type, and compare it against local_class_id
	dj_int_array *arr = (dj_int_array*)ref;
	return (arr->type==typeId.entity_id);
}

char dj_global_id_testType(void *ref, dj_local_id localClassId)
{
	dj_global_id refClass, testClass;
    char result;
    DEBUG_ENTER_NEST(DBG_DARJEELING, "dj_global_id_testType()");

	// determine if the reference on the stack is an array or object
	switch (dj_mem_getChunkId(ref))
	{

        case CHUNKID_INTARRAY:
            DEBUG_LOG(DBG_DARJEELING, "INTARRAY\n");
            result = dj_global_id_checkIntArrayType(ref, localClassId);
            DEBUG_EXIT_NEST(DBG_DARJEELING, "dj_global_id_testType()"); \
            return result;


        case CHUNKID_REFARRAY:
            DEBUG_LOG(DBG_DARJEELING, "REFARRAY\n");

            // resolve the class we're testing against
            testClass = dj_global_id_resolve(dj_exec_getCurrentInfusion(), localClassId);

            // resolve array type
            refClass = dj_vm_getRuntimeClass(((dj_ref_array*)ref)->runtime_class_id);

            result = dj_global_id_testClassType(refClass, testClass);
            DEBUG_EXIT_NEST(DBG_DARJEELING, "dj_global_id_testType()"); \
            return result;

        default:
            DEBUG_LOG(DBG_DARJEELING, "default case\n");

            // resolve the class we're testing against
            testClass = dj_global_id_resolve(dj_exec_getCurrentInfusion(), localClassId);

            // resolve runtime class
            refClass = dj_vm_getRuntimeClass(dj_mem_getChunkId(ref));

            result= dj_global_id_testClassType(refClass, testClass);
            DEBUG_EXIT_NEST(DBG_DARJEELING, "dj_global_id_testType()"); \
            return result;
	}

}

/**
 * Test if a class equals java.lang.Object. Used when iterating over inheritance lists.
 * @param global_class_id the class to test against
 * @return true is the class is java.lang.Object, false otherwise
 */
char dj_global_id_isJavaLangObject(dj_global_id global_class_id)
{
	// TODO cache a dj_global_id object that represents java.lang.Object and compare against that
	char result = (global_class_id.infusion == dj_vm_getSystemInfusion(dj_exec_getVM()))
		&& (global_class_id.entity_id == BASE_CDEF_java_lang_Object);

    DEBUG_LOG(DBG_DARJEELING, "isJavaLangObject({%p,%d}): %s\n",
              global_class_id.infusion,global_class_id.entity_id,(result?"true":"false"));

    return result;
}

/**
 * Gets the runtime class id of a class.
 * @param global_class_id the class to get a runtime id for
 * @return the runtime id for the class
 */
runtime_id_t dj_global_id_getRuntimeClassId(dj_global_id global_class_id)
{
	return global_class_id.infusion->class_base + global_class_id.entity_id;
}

dj_global_id dj_global_id_lookupMethodImplInClassDef(dj_infusion *infusion, dj_local_id methodDefLocalId, dj_di_pointer classDef) {
	dj_global_id ret;
	ret.infusion = NULL;

	// get method table for the current class
	dj_di_pointer methodTable = dj_di_classDefinition_getMethodTable(classDef);

	// scan the method table for the desired method
	for (int i=0; i<dj_di_methodTable_getSize(methodTable); i++)
	{
		dj_di_pointer methodTableEntry = dj_di_methodTable_getEntry(methodTable, i);

		DEBUG_LOG(DBG_DARJEELING, "Method lookup: %d.%d ?= %d.%d\n",
				dj_di_methodTableEntry_getDefinitionEntity(methodTableEntry),
				dj_di_methodTableEntry_getDefinitionInfusion(methodTableEntry),
				methodDefLocalId.entity_id, methodDefLocalId.infusion_id
				);

		if ( (dj_di_methodTableEntry_getDefinitionEntity(methodTableEntry)==methodDefLocalId.entity_id) &&
			(dj_di_methodTableEntry_getDefinitionInfusion(methodTableEntry)==methodDefLocalId.infusion_id) )
		{
			// found method, resolve method definition
			ret = dj_global_id_resolve(infusion, dj_di_methodTableEntry_getImplementation(methodTableEntry));

#ifdef DARJEELING_DEBUG
				char name[16];

				dj_infusion_getName(ret.infusion, name, 16);
				DEBUG_LOG(DBG_DARJEELING, "Found %s %d\n", name, ret.entity_id);
#endif

			return ret;
		}
	}

	return ret;
}

// TODO holy zombiejesus this is ugly
dj_global_id dj_global_id_lookupVirtualMethod(dj_global_id resolvedMethodDefId, dj_object *object)
{
	dj_global_id ret;
	dj_di_pointer classDef;
	dj_global_id classId;
	dj_local_id methodDefLocalId;

	// mark not found
	ret.infusion = NULL;

	// resolve runtime class of the object
	classId = dj_vm_getRuntimeClass(dj_mem_getChunkId(object));

	while (true)
	{
		// Map the resolved method ID to the global ID space of the class' infusion.
		// We will check if this global ID is found in the class' method table.
		methodDefLocalId = dj_global_id_mapToInfusion(resolvedMethodDefId, classId.infusion);

		// get class definition for the class we're scanning
		classDef = dj_global_id_getClassDefinition(classId);

		ret = dj_global_id_lookupMethodImplInClassDef(classId.infusion, methodDefLocalId, classDef);

		// if we found a method at this point, we don't need to keep looking in
		// parent classes
		if (ret.infusion!=NULL) break;

		DEBUG_LOG(DBG_DARJEELING, "Check parent (entity %d, infusion %p)\n", ret.entity_id, ret.infusion);

		// go into the parent class
		if (dj_global_id_isJavaLangObject(classId))
			break;
		else
			classId = dj_global_id_resolve(classId.infusion, dj_di_classDefinition_getSuperClass(classDef));
	}

	return ret;
}

dj_global_id dj_global_id_lookupAnyVirtualMethod(dj_global_id resolvedMethodDefId)
{
	dj_global_id ret;
	// mark not found
	ret.infusion = NULL;

	dj_infusion *infusion = currentVm->infusions;

	while (infusion != NULL)
	{
		// Map the resolved method ID to the global ID space of the class' infusion.
		// We will check if this global ID is found in any class' method table.
		dj_local_id methodDefLocalId = dj_global_id_mapToInfusion(resolvedMethodDefId, infusion);

		for (int i=0; i<dj_infusion_getNumberOfClassDefinitions(infusion); i++) {
			// get class definition for the class we're scanning
			dj_di_pointer classDef = dj_infusion_getClassDefinition(infusion, i);

			ret = dj_global_id_lookupMethodImplInClassDef(infusion, methodDefLocalId, classDef);

			if (ret.infusion!=NULL) {
				return ret;
			}
		}
		infusion = infusion->next;
	}
	
	return ret;
}

/**
 * Gets a method implementation for a global id
 * @param class a global id pointing to a method imlementation
 * @return a pointer to the referenced method implementation
 */
dj_di_pointer dj_global_id_getMethodImplementation(dj_global_id gid)
{
	return dj_infusion_getMethodImplementation(gid.infusion, gid.entity_id);
}

/**
 * Gets a string for a global id
 * @param class a global id pointing to a string
 * @return a pointer to the referenced string
 */
dj_di_pointer dj_global_id_getString(dj_global_id gid)
{
	return dj_infusion_getString(gid.infusion, gid.entity_id);
}

