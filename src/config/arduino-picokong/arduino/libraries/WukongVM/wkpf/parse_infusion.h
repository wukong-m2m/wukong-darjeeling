/*
 * parse_infusion.h
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
 
#ifndef __parse_infusion__
#define __parse_infusion__

#include "types.h"
#include "program_mem.h"

enum ElementType
{
	HEADER = 0,
	CLASSLIST = 1,
	METHODIMPLLIST = 2,
	STATICFIELDINFO = 3,
	METHODDEFLIST = 4,
	INFUSIONLIST = 5,
	STRINGTABLE = 6
};

enum MethodImplementationFlags
{
	FLAGS_NATIVE = 1,
	FLAGS_STATIC = 2
};

enum JavaTypeID
{
	JTID_VOID = 0,
	JTID_BOOLEAN = 4,
	JTID_CHAR = 5,
	JTID_FLOAT = 6,
	JTID_DOUBLE = 7,
	JTID_BYTE = 8,
	JTID_SHORT = 9,
	JTID_INT = 10,
	JTID_LONG = 11,
	JTID_REF = 255
};

#define INTARRAY_INFUSION_ID 254

#define dj_di_getListPointer(pointer, index) (dj_di_getU16( (dj_di_pointer)pointer + 1 + index * 2) )
#define dj_di_getListSize(pointer) (dj_di_getU8( (dj_di_pointer)pointer))

#define dj_di_getListElement(pointer, index) ((dj_di_pointer)((dj_di_pointer)pointer \
	+ dj_di_getListPointer((dj_di_pointer)pointer, index)))

#define dj_di_parentElement_getListSize(pointer) (dj_di_getListSize( (pointer+1) ))
#define dj_di_parentElement_getChild(pointer, index) ( ((dj_di_pointer)(pointer)) \
	+ dj_di_getListPointer( (pointer+1) , index) )

// getters for the Element record
#define dj_di_element_getId(pointer) dj_di_getU8(pointer + 0)

// getters for the StaticFieldInfo record
#define dj_di_staticFieldInfo_getNrRefs(pointer) dj_di_getU8(pointer + 1)
#define dj_di_staticFieldInfo_getNrBytes(pointer) dj_di_getU8(pointer + 2)
#define dj_di_staticFieldInfo_getNrShorts(pointer) dj_di_getU8(pointer + 3)
#define dj_di_staticFieldInfo_getNrInts(pointer) dj_di_getU8(pointer + 4)
#define dj_di_staticFieldInfo_getNrLongs(pointer) dj_di_getU8(pointer + 5)

// getters for the Header record
#define dj_di_header_getMajorVersion(pointer) dj_di_getU8(pointer + 1)
#define dj_di_header_getMinorVersion(pointer) dj_di_getU8(pointer + 2)
#define dj_di_header_getEntryPoint(pointer) dj_di_getU8(pointer + 3)
#define dj_di_header_getInfusionName(pointer) (pointer + 4)

// getters for the infusionList record
#define dj_di_infusionList_getSize(pointer) (dj_di_getListSize((pointer) + 1))
#define dj_di_infusionList_getChild(pointer, index) ( ((dj_di_pointer)pointer) +\
		dj_di_getListPointer(((dj_di_pointer)pointer)+1, index) +1 )

// getters for the Method Definition List
#define dj_di_methodTableEntry_getDefinitionInfusion(pointer) dj_di_getU8(pointer + 0)
#define dj_di_methodTableEntry_getDefinitionEntity(pointer) dj_di_getU8(pointer + 1)
#define dj_di_methodTableEntry_getImplementationInfusion(pointer) dj_di_getU8(pointer + 2)
#define dj_di_methodTableEntry_getImplementationEntity(pointer) dj_di_getU8(pointer + 3)
#define dj_di_methodTableEntry_getDefinition(pointer) (dj_di_getLocalId(pointer+0))
#define dj_di_methodTableEntry_getImplementation(pointer) (dj_di_getLocalId(pointer+2))

// getters for the Method Definition List
#define dj_di_methodTable_getSize(pointer) dj_di_getU8((pointer) + 0)
#define dj_di_methodTable_getEntry(pointer, index) (((pointer) + 1 + index * 4))

// Class definition getters
#define dj_di_classDefinition_getNrRefs(pointer) dj_di_getU8(pointer + 0)
#define dj_di_classDefinition_getOffsetOfFirstReference(pointer) dj_di_getU8(pointer + 1)
#define dj_di_classDefinition_getSuperClass(pointer) (dj_di_getLocalId(pointer+2))
#define dj_di_classDefinition_getCLInit(pointer) (dj_di_getU8(pointer+4))
#define dj_di_classDefinition_getClassName(pointer) (dj_di_getLocalId(pointer + 5))
#define dj_di_classDefinition_getNrInterfaces(pointer) dj_di_getU8(pointer + 7)
#define dj_di_classDefinition_getInterface(pointer, index) (dj_di_getLocalId(pointer + 8 + index * 2))
#define dj_di_classDefinition_getMethodTable(pointer) ((dj_di_pointer)(pointer) + 8 + dj_di_classDefinition_getNrInterfaces(pointer) * 2)

// method implementation getters
#define dj_di_methodImplementation_getReferenceArgumentCount(pointer) dj_di_getU8(pointer + 0)
#define dj_di_methodImplementation_getIntegerArgumentCount(pointer) dj_di_getU8(pointer + 1)
#define dj_di_methodImplementation_getReferenceLocalVariableCount(pointer) dj_di_getU8(pointer + 2)
#define dj_di_methodImplementation_getIntegerLocalVariableCount(pointer) dj_di_getU8(pointer + 3)
#define dj_di_methodImplementation_getParameterCount(pointer) dj_di_getU8(pointer + 4)
#define dj_di_methodImplementation_getMaxStack(pointer) dj_di_getU8(pointer + 5)
#define dj_di_methodImplementation_getFlags(pointer) dj_di_getU8(pointer + 6)
#define dj_di_methodImplementation_getReturnType(pointer) dj_di_getU8(pointer + 7)
#define dj_di_methodImplementation_getLength(pointer) dj_di_getU16(pointer + 8)
#define dj_di_methodImplementation_getData(pointer) (pointer + 10)

#define dj_di_methodImplementation_getNrExceptionHandlers(pointer) \
	dj_di_getU8((pointer + 10 + dj_di_getU16(pointer + 8)))

#define dj_di_methodImplementation_getExceptionHandlerType(pointer, i) \
	dj_di_getLocalId((pointer + 11 + dj_di_getU16(pointer + 8) + i * 8))

#define dj_di_methodImplementation_getExceptionHandlerStartPC(pointer, i) \
	dj_di_getU16((pointer + 11 + dj_di_getU16(pointer + 8) + i * 8 + 2))

#define dj_di_methodImplementation_getExceptionHandlerEndPC(pointer, i) \
	dj_di_getU16((pointer + 11 + dj_di_getU16(pointer + 8) + i * 8 + 4))

#define dj_di_methodImplementation_getExceptionHandlerCatchPC(pointer, i) \
	dj_di_getU16((pointer + 11 + dj_di_getU16(pointer + 8) + i * 8 + 6))

// string table getters
#define dj_di_stringtable_getNrElements(pointer) dj_di_getU16(pointer + 1)
#define dj_di_stringtable_getElementLength(pointer, i) dj_di_getU16(pointer + dj_di_getU16(pointer+3+2*i))
#define dj_di_stringtable_getElementBytes(pointer, i) (pointer + dj_di_getU16(pointer+3+2*i) + 2)

#endif
