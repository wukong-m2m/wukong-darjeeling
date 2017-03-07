/*
 * javax_darjeeling_actuators_Leds.c
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

#define NUM_VIRTUAL_LEDS 3

//short javax.darjeeling.actuators.Leds.getNrLeds()
void javax_darjeeling_actuators_Leds_short_getNrLeds()
{
	dj_exec_stackPushShort(NUM_VIRTUAL_LEDS);
}

// void javax.darjeeling.actuators.Leds.set(short, boolean)
void javax_darjeeling_actuators_Leds_void_set_short_boolean()
{
	uint16_t on = dj_exec_stackPopShort();
	int16_t nr = dj_exec_stackPopShort();

	// Check for out-of-bounds
	if (nr>=0 && nr<NUM_VIRTUAL_LEDS)
		printf(on?"LED %d ON(on/off=%d)\n":"LED %d OFF(on/off=%d)\n", nr, on);
	else
		dj_exec_createAndThrow(BASE_CDEF_java_lang_IndexOutOfBoundsException);

}
