/*
 * javax_darjeeling_io_ProgramFlash.c
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
 
#include <stdio.h>

#include "array.h"
#include "execution.h"
#include "jlib_base.h"
#include "config.h"


// short javax.darjeeling.io.ProgramFlash.getBlockSize()
void javax_darjeeling_io_ProgramFlash_short_getBlockSize()
{
	dj_exec_stackPushShort((uint16_t)PROGFLASH_BLOCKSIZE);
	/*ref_t obj = */dj_exec_stackPopRef();
}

// int javax.darjeeling.io.ProgramFlash.getBlockCount()
void javax_darjeeling_io_ProgramFlash_int_getBlockCount()
{
	dj_exec_stackPushInt((uint32_t)PROGFLASH_BLOCKCOUNT);
	/*ref_t obj =*/ dj_exec_stackPopRef();
}

// byte[] javax.darjeeling.io.ProgramFlash.read(int)
void javax_darjeeling_io_ProgramFlash_byte____read_int()
{
	// Pop arguments.
	/*uint32_t index =*/ dj_exec_stackPopInt();
	/*ref_t obj =*/ dj_exec_stackPopRef();

	// Allocate byte array.
	dj_int_array * arr = dj_int_array_create(T_BYTE, PROGFLASH_BLOCKSIZE);

	// Read data from 'program flash'.

	// TODO implement ...

	// Return data.
	dj_exec_stackPushRef(VOIDP_TO_REF(arr));
}

// void javax.darjeeling.io.ProgramFlash.write(int, byte[])
void javax_darjeeling_io_ProgramFlash_void__write_int_byte__()
{
	// Pop arguments.
	/*dj_int_array * arr =*/ REF_TO_VOIDP(dj_exec_stackPopRef());
	/*uint32_t index =*/ dj_exec_stackPopInt();
	/*ref_t obj =*/ dj_exec_stackPopRef();

	// Write data to 'program flash'.

	// TODO implement ...
}

