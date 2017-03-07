/*
 * javax_darjeeling_sensors_Buttons.c
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

#include "array.h"
#include "execution.h"
#include "jlib_base.h"

void javax_darjeeling_sensors_Buttons_short_getNrButtons()
{
	dj_exec_stackPushShort(0);
}

// boolean javax.darjeeling.sensors.Buttons.pressed(short)
void javax_darjeeling_sensors_Buttons_boolean_pressed_short()
{
	dj_exec_stackPushShort(0);
}

// void javax.darjeeling.sensors.Buttons.waitForDown(short)
void javax_darjeeling_sensors_Buttons_void_waitForDown_short()
{
	// Do nothing.
}

// void javax.darjeeling.sensors.Buttons.waitForUp(short)
void javax_darjeeling_sensors_Buttons_void_waitForUp_short()
{
	// Do nothing.
}

