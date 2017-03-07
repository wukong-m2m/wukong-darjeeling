/*
 * javax_darjeeling_io_Serial.c
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
#include "conio.h"	// getchar is, exceptionally, not defined in stdio.h

#include "array.h"
#include "execution.h"
#include "jlib_base.h"

// void javax.darjeeling.io.Serial.setBaudRate(byte, int)
void javax_darjeeling_io_Serial_void_setBaudRate_byte_int()
{
	dj_exec_stackPopInt();
	dj_exec_stackPopShort();

	// Do nothing.
}

// byte javax.darjeeling.io.Serial.getNrSerialPorts()
void javax_darjeeling_io_Serial_byte_getNrSerialPorts()
{
	dj_exec_stackPushShort(1);
}

// int javax.darjeeling.io.Serial.getDefaultBaudRate(byte)
void javax_darjeeling_io_Serial_int_getDefaultBaudRate_byte()
{
	dj_exec_stackPopShort();
	dj_exec_stackPushInt(57600);
}

// byte javax.darjeeling.io.SerialInputStream._read(byte)
// NOTE: this implementation is blocking, meaning that the entire JVM is blocked when you
// read from the 'serial port'. Should be replaced by callback based method.
void javax_darjeeling_io_SerialInputStream_short__read_byte()
{
	dj_exec_stackPopShort();
	dj_exec_stackPushShort((uint16_t) getchar() );
}

// short javax.darjeeling.io.SerialInputStream._waitForByte(byte)
void javax_darjeeling_io_SerialInputStream_void__waitForByte_byte()
{
	dj_exec_stackPopShort();

	// Don't do anything, read is implemented blocking
}

// void javax.darjeeling.io.SerialOutputStream._write(byte, int)
void javax_darjeeling_io_SerialOutputStream_void__write_byte_int()
{
	/*int32_t b =*/ dj_exec_stackPopShort();
	dj_exec_stackPopShort();

//	avr_serialWrite(b);
}
