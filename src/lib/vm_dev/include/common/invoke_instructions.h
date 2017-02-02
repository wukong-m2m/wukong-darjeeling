/*
 * invoke_instructions.h
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

#include "execution_instructions.h"

/**
 * Return from function
 */
static inline void RETURN()
{
	// return
	returnFromMethod();
}

/**
 * Return from short/byte/boolean/char function
 */
static inline void SRETURN()
{
	// pop return value off the stack
	int16_t ret = popShort();

	// return
	returnFromMethod();

	// push return value on the runtime stack
	pushShort(ret);

}

/**
 * Return from int function
 */
static inline void IRETURN()
{

	// pop return value off the stack
	int32_t ret = popInt();

	// return
	returnFromMethod();

	// push return value on the runtime stack
	pushInt(ret);
}

/**
 * Return from long function
 */
static inline void LRETURN()
{

	// pop return value off the stack
	int64_t ret = popLong();

	// return
	returnFromMethod();

	// push return value on the runtime stack
	pushLong(ret);
}

static inline void ARETURN()
{

	// pop return value off the stack
	ref_t ret = popRef();

	// return
	returnFromMethod();

	// push return value on the runtime stack
	pushRef(ret);
}


static inline void INVOKESTATIC()
{
	dj_local_id localId = dj_fetchLocalId();
	dj_global_id globalId = dj_global_id_resolve(dj_exec_getCurrentInfusion(),  localId);
	callMethod(globalId, false);
}


static inline void INVOKESPECIAL()
{
	dj_local_id localId = dj_fetchLocalId();
	dj_global_id globalId = dj_global_id_resolve(dj_exec_getCurrentInfusion(),  localId);

	callMethod(globalId, true);
}

static inline void INVOKEVIRTUAL()
{
	// fetch the method definition's global id and resolve it
	dj_local_id dj_local_id = dj_fetchLocalId();

	// fetch the number of arguments for the method.
	uint8_t nr_ref_args = fetch();

	DO_INVOKEVIRTUAL(dj_local_id, nr_ref_args);
}

static inline void INVOKEINTERFACE()
{
	INVOKEVIRTUAL();
}

#endif // EXECUTION_DISABLEINTERPRETER_COMPLETELY
