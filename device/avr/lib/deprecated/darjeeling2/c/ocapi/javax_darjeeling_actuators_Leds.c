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
#include "gpio.h"

#if (BOARD==MATRIXONE)
#define NUM_LEDS 4
static int portNum[] = {4, 4, 4, 4, 5, 5};
static uint8_t bitNum[] = {0x08, 0x10, 0x20, 0x40, 0x10, 0x20};
#define LED_ENABLE_IDX		4
#define LED_GROUP_SEL_IDX	5

#elif (BOARD==EXPERIMENTER)
#define NUM_LEDS 2
static int portNum[] = {1, 1};
static uint8_t bitNum[] = {0x01, 0x02};

#elif (BOARD==FEUERWHERE)
#define NUM_LEDS 8
static int portNum[] = {8, 8, 8, 8, 8, 8, 8, 8};
static uint8_t bitNum[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

#else
#define NUM_LEDS 0
static int portNum[] = {};
static uint8_t bitNum[] = {};
#endif


void initLed() {
	int idx=0;

#if (BOARD==MATRIXONE)
	gpioConfig( portNum[LED_ENABLE_IDX], bitNum[LED_ENABLE_IDX], gpioConfigOutputReduced );
	gpioConfig( portNum[LED_GROUP_SEL_IDX], bitNum[LED_GROUP_SEL_IDX], gpioConfigOutputReduced );
	gpioSetBits(portNum[LED_ENABLE_IDX], bitNum[LED_ENABLE_IDX]);		// LED enable
	gpioSetBits(portNum[LED_GROUP_SEL_IDX], bitNum[LED_GROUP_SEL_IDX]);	// Group A = LED 1,3,5,7
#endif

	for( idx=0; idx < NUM_LEDS; idx++ ) {
		gpioConfig( portNum[idx], bitNum[idx], gpioConfigOutputFull );
	}
	for( idx=0; idx < NUM_LEDS; idx++ ) {
		gpioClearBits(portNum[idx], bitNum[idx]);
	}
}

//short javax.darjeeling.actuators.Leds.getNrLeds()
void javax_darjeeling_actuators_Leds_short_getNrLeds()
{
	dj_exec_stackPushShort(NUM_LEDS);
}

// void javax.darjeeling.actuators.Leds.set(short, boolean)
void javax_darjeeling_actuators_Leds_void_set_short_boolean()
{
	uint16_t on = dj_exec_stackPopShort();
	// Get the led index argument
	uint16_t nr = dj_exec_stackPopShort();
	if( nr < NUM_LEDS ) {
		if( on ) {
			gpioSetBits( portNum[nr], bitNum[nr] );
		} else {
			gpioClearBits( portNum[nr], bitNum[nr] );
		}
	}
}
