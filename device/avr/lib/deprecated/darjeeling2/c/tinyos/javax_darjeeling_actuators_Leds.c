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

//declarations of nesc functions
void nesc_setLed(int nr, int on);
int32_t nesc_getTime();
int32_t nesc_getNodeId();
uint16_t nesc_getMaxPayloadLength();
int nesc_send(const char * message, int16_t receiverId, uint16_t length);
uint16_t nesc_peekMessageLength();
void * nesc_popMessageBuffer();
int nesc_getNrMessages();
int nesc_wasAcked();

//short javax.darjeeling.actuators.Leds.getNrLeds()
void javax_darjeeling_actuators_Leds_short_getNrLeds()
{
	dj_exec_stackPushShort(NUM_VIRTUAL_LEDS);
}

// void javax.darjeeling.actuators.Leds.set(short, boolean)
void javax_darjeeling_actuators_Leds_void_set_short_boolean()
{
	uint16_t on = dj_exec_stackPopShort();
	uint16_t nr = dj_exec_stackPopShort();

	// Check for out-of-bounds
	if (nr>=0 && nr<NUM_VIRTUAL_LEDS)
		nesc_setLed(nr, on);
	else
		dj_exec_createAndThrow(BASE_CDEF_java_lang_IndexOutOfBoundsException);
}
