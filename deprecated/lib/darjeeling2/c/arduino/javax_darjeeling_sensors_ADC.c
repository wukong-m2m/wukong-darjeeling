/*
 * javax_darjeeling_sensors_ADC.c
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

// short javax.darjeeling.sensors.ADC.getNrADCs()
void javax_darjeeling_sensors_ADC_short_getNrADCs()
{
	// No ADCs.
	dj_exec_stackPushShort(0);
}

// int javax.darjeeling.sensors.ADC.read(short)
void javax_darjeeling_sensors_ADC_int_read_short()
{
	dj_exec_stackPopShort();

	// No ADCs.
	dj_exec_createAndThrow(BASE_CDEF_java_lang_IndexOutOfBoundsException);
}

// byte javax.darjeeling.sensors.ADC.getResolution(short)
void javax_darjeeling_sensors_ADC_byte_getResolution_short()
{
	// No ADCs.
	dj_exec_createAndThrow(BASE_CDEF_java_lang_IndexOutOfBoundsException);
}
