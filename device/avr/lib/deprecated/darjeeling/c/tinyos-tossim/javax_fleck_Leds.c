/*
 * javax_fleck_Leds.c
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
 
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "execution.h"
#include "jlib_base.h"
#include "heap.h"
#include "types.h"
#include "tossim.h"
// void javax.fleck.Leds.setLed(int, boolean)
void javax_fleck_Leds_void_setLed_int_boolean()
{
	unsigned char on = dj_exec_stackPopShort();
	unsigned int nr = dj_exec_stackPopInt();
	char * tempStr = dj_mem_alloc(25, CHUNKID_REFARRAY);
	snprintf(tempStr, 25, on?"LED %d ON(on/off=%d)\n":"LED %d OFF(on/off=%d)\n", nr, on);
	tossim_printf(tempStr);
	dj_mem_free(tempStr);
	
}
