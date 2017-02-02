/*
 * array_instructions.h
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
 
#ifndef EXECUTION_DISABLEINTERPRETER_COMPLETELY

/**
 * Executes the NEWARRAY instruction. The array type is fetched from code (uint8_t) and the length is
 * popped from the stack. Note that only types T_BOOLEAN, T_BYTE, T_SHORT and T_INT are currently
 * supported. An array of references is created with the ANEWARRAY instruction.
 *
 */
static inline void NEWARRAY()
{
	dj_int_array *arr = dj_int_array_create(fetch(), popShort());

	if (arr==nullref)
		dj_exec_createAndThrow(OUTOFMEMORY_ERROR);
	else
		pushRef(VOIDP_TO_REF(arr));
}

static inline void ANEWARRAY()
{
	dj_local_id classLocalId = dj_fetchLocalId();
	uint16_t size = popShort();

	ref_t arr = DO_ANEWARRAY(classLocalId, size);
	if (arr!=nullref)
		pushRef(arr);
}

/**
 * Executes the ARRAYLENGTH instruction. A reference to an array is popped from the stack, and the length
 * of that array is pushed into the stack.
 *
 */
static inline void ARRAYLENGTH()
{
	dj_array *array = REF_TO_VOIDP(popRef());
	if (array==NULL)
		dj_exec_createAndThrow(NULLPOINTER_EXCEPTION);
	else
		pushShort(array->length);
}

/**
 * Executes the BALOAD instruction. An index and array are popped from the stack. The byte value in the
 * array at index is then pushed on the stack. When index is out of bounds, (outside
 * the [0..size-1] range) throwOutOfBoundsException() is called to handle the exception.
 */
static inline void BALOAD()
{
#ifdef ARRAYINDEX_32BIT
	int32_t index = popInt();
#else
	int16_t index = popShort();
#endif
	dj_int_array *arr = REF_TO_VOIDP(popRef());

	if (arr==NULL)
		dj_exec_createAndThrow(NULLPOINTER_EXCEPTION);
	else
		if ((index>=0) && (index<((dj_array*)arr)->length))
			pushShort((int16_t)arr->data.bytes[index]);
		else
			dj_exec_createAndThrow(INDEXOUTOFBOUNDS_EXCEPTION);

}

/**
 * Executes the CALOAD instruction. An index and array are popped from the stack. The char value in the
 * array at index is then pushed on the stack. When index is out of bounds, (outside
 * the [0..size-1] range) throwOutOfBoundsException() is called to handle the exception.
 */
static inline void CALOAD()
{
	BALOAD();
}

/**
 * Executes the SALOAD instruction. An index and array are popped from the stack. The short value in the
 * array at index is then pushed on the stack. When index is out of bounds, (outside
 * the [0..size-1] range) throwOutOfBoundsException() is called to handle the exception.
 */
static inline void SALOAD()
{
#ifdef ARRAYINDEX_32BIT
	int32_t index = popInt();
#else
	int16_t index = popShort();
#endif
	dj_int_array *arr = REF_TO_VOIDP(popRef());

	if (arr==NULL)
		dj_exec_createAndThrow(NULLPOINTER_EXCEPTION);
	else
		if ((index>=0) && (index<((dj_array*)arr)->length))
			pushShort((int16_t)arr->data.shorts[index]);
		else
			dj_exec_createAndThrow(INDEXOUTOFBOUNDS_EXCEPTION);

}

/**
 * Executes the IALOAD instruction. An index and array are popped from the stack. The integer value in the
 * array at index is then pushed on the stack. When index is out of bounds, (outside
 * the [0..size-1] range) throwOutOfBoundsException() is called to handle the exception.
 */
static inline void IALOAD()
{
#ifdef ARRAYINDEX_32BIT
	int32_t index = popInt();
#else
	int16_t index = popShort();
#endif
	dj_int_array *arr = REF_TO_VOIDP(popRef());

	if (arr==NULL)
		dj_exec_createAndThrow(NULLPOINTER_EXCEPTION);
	else
		if ((index>=0) && (index<((dj_array*)arr)->length))
			pushInt((int32_t)arr->data.ints[index]);
		else
			dj_exec_createAndThrow(INDEXOUTOFBOUNDS_EXCEPTION);
}

/**
 * Executes the LALOAD instruction. An index and array are popped from the stack. The long value in the
 * array at index is then pushed on the stack. When index is out of bounds, (outside
 * the [0..size-1] range) throwOutOfBoundsException() is called to handle the exception.
 */
static inline void LALOAD()
{
#ifdef ARRAYINDEX_32BIT
	int32_t index = popInt();
#else
	int16_t index = popShort();
#endif
	dj_int_array *arr = REF_TO_VOIDP(popRef());

	if (arr==NULL)
		dj_exec_createAndThrow(NULLPOINTER_EXCEPTION);
	else
		if ((index>=0) && (index<((dj_array*)arr)->length))
			pushLong((int64_t)arr->data.longs[index]);
		else
			dj_exec_createAndThrow(INDEXOUTOFBOUNDS_EXCEPTION);
}


static inline void AALOAD()
{
#ifdef ARRAYINDEX_32BIT
	int32_t index = popInt();
#else
	int16_t index = popShort();
#endif
	dj_ref_array *arr = REF_TO_VOIDP(popRef());

	if (arr==NULL)
		dj_exec_createAndThrow(NULLPOINTER_EXCEPTION);
	else
		if ((index>=0) && (index<((dj_array*)arr)->length))
			pushRef(arr->refs[index]);
		else
			dj_exec_createAndThrow(INDEXOUTOFBOUNDS_EXCEPTION);
}

/**
 * Executes the BASTORE instruction. A byte value, index and array reference are popped from the stack.
 * The byte value is then stored in the array at index. When index is out of bounds,  (outside
 * the [0..size-1] range) throwOutOfBoundsException() is called to handle the exception.
 */
static inline void BASTORE()
{
	uint8_t value = popShort();
#ifdef ARRAYINDEX_32BIT
	int32_t index = popInt();
#else
	int16_t index = popShort();
#endif
	dj_int_array *arr = REF_TO_VOIDP(popRef());
	if (arr==NULL)
		dj_exec_createAndThrow(NULLPOINTER_EXCEPTION);
	else
		if ((index>=0) && (index<((dj_array*)arr)->length))
			arr->data.bytes[index] = value;
		else
			dj_exec_createAndThrow(INDEXOUTOFBOUNDS_EXCEPTION);
}

/**
 * Executes the CASTORE instruction. A char value, index and array reference are popped from the stack.
 * The char value is then stored in the array at index. When index is out of bounds,  (outside
 * the [0..size-1] range) throwOutOfBoundsException() is called to handle the exception.
 */
static inline void CASTORE()
{
	BASTORE();
}

/**
 * Executes the SASTORE instruction. A short value, index and array reference are popped from the stack.
 * The short value is then stored in the array at index. When index is out of bounds,  (outside
 * the [0..size-1] range) throwOutOfBoundsException() is called to handle the exception.
 */
static inline void SASTORE()
{
	int16_t value = popShort();
#ifdef ARRAYINDEX_32BIT
	int32_t index = popInt();
#else
	int16_t index = popShort();
#endif
	dj_int_array *arr = REF_TO_VOIDP(popRef());

	if (arr==NULL)
		dj_exec_createAndThrow(NULLPOINTER_EXCEPTION);
	else
		if ((index>=0) && (index<((dj_array*)arr)->length))
			arr->data.shorts[index] = value;
		else
			dj_exec_createAndThrow(INDEXOUTOFBOUNDS_EXCEPTION);
}

/**
 * Executes the IASTORE instruction. An integer value, index and array reference are popped from the stack.
 * The integer value is then stored in the array at index. When index is out of bounds,  (outside
 * the [0..size-1] range) throwOutOfBoundsException() is called to handle the exception.
 */
static inline void IASTORE()
{
	int32_t value = popInt();
#ifdef ARRAYINDEX_32BIT
	int32_t index = popInt();
#else
	int16_t index = popShort();
#endif
	dj_int_array *arr = REF_TO_VOIDP(popRef());

	if (arr==NULL)
		dj_exec_createAndThrow(NULLPOINTER_EXCEPTION);
	else
		if ((index>=0) && (index<((dj_array*)arr)->length))
			arr->data.ints[index] = value;
		else
			dj_exec_createAndThrow(INDEXOUTOFBOUNDS_EXCEPTION);

}

/**
 * Executes the LASTORE instruction. A long value, index and array reference are popped from the stack.
 * The long value is then stored in the array at index. When index is out of bounds,  (outside
 * the [0..size-1] range) throwOutOfBoundsException() is called to handle the exception.
 */
static inline void LASTORE()
{
	int64_t value = popLong();
#ifdef ARRAYINDEX_32BIT
	int32_t index = popInt();
#else
	int16_t index = popShort();
#endif
	dj_int_array *arr = REF_TO_VOIDP(popRef());

	if (arr==NULL)
		dj_exec_createAndThrow(NULLPOINTER_EXCEPTION);
	else
		if ((index>=0) && (index<((dj_array*)arr)->length))
			arr->data.longs[index] = value;
		else
			dj_exec_createAndThrow(INDEXOUTOFBOUNDS_EXCEPTION);

}

/**
 * Executes the IASTORE instruction. An integer value, index and array reference are popped from the stack.
 * The integer value is then stored in the array at index. When index is out of bounds,  (outside
 * the [0..size-1] range) throwOutOfBoundsException() is called to handle the exception.
 */
static inline void AASTORE()
{
	ref_t value = popRef();
#ifdef ARRAYINDEX_32BIT
	int32_t index = popInt();
#else
	int16_t index = popShort();
#endif
	dj_ref_array *arr = REF_TO_VOIDP(popRef());

	if (arr==NULL)
		dj_exec_createAndThrow(NULLPOINTER_EXCEPTION);
	else
		if ((index>=0) && (index<((dj_array*)arr)->length))
			arr->refs[index] = value;
		else
			dj_exec_createAndThrow(INDEXOUTOFBOUNDS_EXCEPTION);

}

#endif // EXECUTION_DISABLEINTERPRETER_COMPLETELY