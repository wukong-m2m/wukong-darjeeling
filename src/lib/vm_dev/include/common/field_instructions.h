/*
 * field_instructions.h
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
 
#include "config.h"


static inline void GETSTATIC_B()
{
	uint8_t infusion_id = fetch();
	dj_infusion *infusion = dj_infusion_resolve(dj_exec_getCurrentInfusion(), infusion_id);
	pushShort((int8_t)infusion->staticByteFields[fetch()]);
}

static inline void GETSTATIC_C()
{
	GETSTATIC_B();
}

static inline void GETSTATIC_S()
{
	uint8_t infusion_id = fetch();
	dj_infusion *infusion = dj_infusion_resolve(dj_exec_getCurrentInfusion(), infusion_id);
	pushShort((int16_t)infusion->staticShortFields[fetch()]);
}

static inline void GETSTATIC_I()
{
	uint8_t infusion_id = fetch();
	dj_infusion *infusion = dj_infusion_resolve(dj_exec_getCurrentInfusion(), infusion_id);
	pushInt(infusion->staticIntFields[fetch()]);
}

static inline void GETSTATIC_L()
{
	uint8_t infusion_id = fetch();
	dj_infusion *infusion = dj_infusion_resolve(dj_exec_getCurrentInfusion(), infusion_id);
	pushLong(infusion->staticLongFields[fetch()]);
}

static inline void GETSTATIC_A()
{
	uint8_t infusion_id = fetch();
	dj_infusion *infusion = dj_infusion_resolve(dj_exec_getCurrentInfusion(), infusion_id);
	pushRef(infusion->staticReferenceFields[fetch()]);
}

static inline void PUTSTATIC_B()
{
	uint8_t infusion_id = fetch();
	dj_infusion *infusion = dj_infusion_resolve(dj_exec_getCurrentInfusion(), infusion_id);
	infusion->staticByteFields[fetch()] = (int8_t)popShort();
}

static inline void PUTSTATIC_C()
{
	PUTSTATIC_B();
}

static inline void PUTSTATIC_S()
{
	uint8_t infusion_id = fetch();
	dj_infusion *infusion = dj_infusion_resolve(dj_exec_getCurrentInfusion(), infusion_id);
	infusion->staticShortFields[fetch()] = (int16_t)popShort();
}

static inline void PUTSTATIC_I()
{
	uint8_t infusion_id = fetch();
	dj_infusion *infusion = dj_infusion_resolve(dj_exec_getCurrentInfusion(), infusion_id);
	infusion->staticIntFields[fetch()] = popInt();
}

static inline void PUTSTATIC_L()
{
	uint8_t infusion_id = fetch();
	dj_infusion *infusion = dj_infusion_resolve(dj_exec_getCurrentInfusion(), infusion_id);
	infusion->staticLongFields[fetch()] = popLong();
}

static inline void PUTSTATIC_A()
{
	uint8_t infusion_id = fetch();
	dj_infusion *infusion = dj_infusion_resolve(dj_exec_getCurrentInfusion(), infusion_id);
	infusion->staticReferenceFields[fetch()] = popRef();
}

static inline void GETFIELD_B()
{
	dj_object *object = REF_TO_VOIDP(popRef());

	if (object==NULL)
		dj_exec_createAndThrow(BASE_CDEF_java_lang_NullPointerException);
	else if (dj_object_getRuntimeId(object)==CHUNKID_INVALID)
		dj_exec_createAndThrow(BASE_CDEF_javax_darjeeling_vm_ClassUnloadedException);
	else {
		uint16_t index = (fetch()<<8) + fetch();
		pushShort( *((int8_t*)((size_t)object+index)) );
	}

}

static inline void GETFIELD_C()
{
	GETFIELD_B();
}

static inline void GETFIELD_S()
{
	dj_object *object = REF_TO_VOIDP(popRef());

	if (object==NULL)
		dj_exec_createAndThrow(BASE_CDEF_java_lang_NullPointerException);
	else if (dj_object_getRuntimeId(object)==CHUNKID_INVALID)
		dj_exec_createAndThrow(BASE_CDEF_javax_darjeeling_vm_ClassUnloadedException);
	else {
		uint16_t index = (fetch()<<8) + fetch();
#ifdef ALIGN_16
		uint8_t* p = ((uint8_t*)((size_t)object+index));
		pushShort( (uint16_t)p[1]<<8 | p[0] );
#else
		pushShort( *((int16_t*)((size_t)object+index)) );
#endif
	}

}

static inline void GETFIELD_I()
{
	dj_object *object = REF_TO_VOIDP(popRef());

	if (object==NULL)
		dj_exec_createAndThrow(BASE_CDEF_java_lang_NullPointerException);
	else if (dj_object_getRuntimeId(object)==CHUNKID_INVALID)
		dj_exec_createAndThrow(BASE_CDEF_javax_darjeeling_vm_ClassUnloadedException);
	else {
		uint16_t index = (fetch()<<8) + fetch();
#ifdef ALIGN_16
		uint8_t* p = ((uint8_t*)((size_t)object+index));
		pushInt( (uint32_t)p[3]<<24 | (uint32_t)p[2]<<16 | (uint32_t)p[1]<<8 | p[0] );
#else
		pushInt( *((int32_t*)((size_t)object+index)) );
#endif
	}

}

static inline void GETFIELD_L()
{
	dj_object *object = REF_TO_VOIDP(popRef());

	if (object==NULL)
		dj_exec_createAndThrow(BASE_CDEF_java_lang_NullPointerException);
	else if (dj_object_getRuntimeId(object)==CHUNKID_INVALID)
		dj_exec_createAndThrow(BASE_CDEF_javax_darjeeling_vm_ClassUnloadedException);
	else {
		uint16_t index = (fetch()<<8) + fetch();
#ifdef ALIGN_16
		uint8_t* p = ((uint8_t*)((size_t)object+index));
		pushLong( (uint64_t)p[7]<<56 | (uint64_t)p[6]<<48 | (uint64_t)p[5]<<40 | (uint64_t)p[4]<<32|
		          (uint64_t)p[3]<<24 | (uint64_t)p[2]<<16 | (uint64_t)p[1]<<8  | p[0] );
#else
		pushLong( *((int64_t*)((size_t)object+index)) );
#endif
	}

}

static inline void GETFIELD_A()
{
	dj_object *object = REF_TO_VOIDP(popRef());

	if (object==NULL)
		dj_exec_createAndThrow(BASE_CDEF_java_lang_NullPointerException);
	else if (dj_object_getRuntimeId(object)==CHUNKID_INVALID)
		dj_exec_createAndThrow(BASE_CDEF_javax_darjeeling_vm_ClassUnloadedException);
	else {
		uint16_t index = (fetch()<<8) + fetch();
		pushRef( dj_object_getReferences(object)[index] );
	}

}


static inline void PUTFIELD_B()
{
	int8_t value = popShort();
	dj_object *object = REF_TO_VOIDP(popRef());

	if (object==NULL)
		dj_exec_createAndThrow(BASE_CDEF_java_lang_NullPointerException);
	else if (dj_object_getRuntimeId(object)==CHUNKID_INVALID)
		dj_exec_createAndThrow(BASE_CDEF_javax_darjeeling_vm_ClassUnloadedException);
	else
	{
		uint16_t index = (fetch()<<8) + fetch();
		*(int8_t*)((char*)object+index) = value;
	}

}

static inline void PUTFIELD_C()
{
	PUTFIELD_B();
}

static inline void PUTFIELD_S()
{
	uint16_t value = popShort();
	dj_object *object = REF_TO_VOIDP(popRef());

	if (object==NULL)
		dj_exec_createAndThrow(BASE_CDEF_java_lang_NullPointerException);
	else if (dj_object_getRuntimeId(object)==CHUNKID_INVALID)
		dj_exec_createAndThrow(BASE_CDEF_javax_darjeeling_vm_ClassUnloadedException);
	else
	{
		uint16_t index = (fetch()<<8) + fetch();
#ifdef ALIGN_16
		uint8_t* p = ((uint8_t*)((size_t)object+index));
                p[1]=value>>8;p[0]=value;
#else
		*(uint16_t*)((size_t)object+index) = value;
#endif
	}
}

static inline void PUTFIELD_I()
{
	uint32_t value = popInt();
	dj_object *object = REF_TO_VOIDP(popRef());

	if (object==NULL)
		dj_exec_createAndThrow(BASE_CDEF_java_lang_NullPointerException);
	else if (dj_object_getRuntimeId(object)==CHUNKID_INVALID)
		dj_exec_createAndThrow(BASE_CDEF_javax_darjeeling_vm_ClassUnloadedException);
	else
	{
		uint16_t index = (fetch()<<8) + fetch();
#ifdef ALIGN_16
		uint8_t* p = ((uint8_t*)((size_t)object+index));
                p[3]=value>>24;p[2]=value>>16;
                p[1]=value>>8; p[0]=value;
#else
		*(uint32_t*)((size_t)object+index) = value;
#endif
	}

}

static inline void PUTFIELD_L()
{
	uint64_t value = popLong();
	dj_object *object = REF_TO_VOIDP(popRef());

	if (object==NULL)
		dj_exec_createAndThrow(BASE_CDEF_java_lang_NullPointerException);
	else if (dj_object_getRuntimeId(object)==CHUNKID_INVALID)
		dj_exec_createAndThrow(BASE_CDEF_javax_darjeeling_vm_ClassUnloadedException);
	else
	{
		uint16_t index = (fetch()<<8) + fetch();
#ifdef ALIGN_16
		uint8_t* p = ((uint8_t*)((size_t)object+index));
                p[7]=value>>56;p[6]=value>>48;
                p[5]=value>>40;p[4]=value>>32;
                p[3]=value>>24;p[2]=value>>16;
                p[1]=value>>8; p[0]=value;
#else
		*(uint64_t*)((size_t)object+index) = value;
#endif
	}

}

static inline void PUTFIELD_A()
{
	ref_t value = popRef();
	dj_object *object = REF_TO_VOIDP(popRef());

	if (object==NULL)
		dj_exec_createAndThrow(BASE_CDEF_java_lang_NullPointerException);
	else if (dj_object_getRuntimeId(object)==CHUNKID_INVALID)
		dj_exec_createAndThrow(BASE_CDEF_javax_darjeeling_vm_ClassUnloadedException);
	else
	{
		uint16_t index = (fetch()<<8) + fetch();
		dj_object_getReferences(object)[index] = value;
	}

}
