/*
 * javax_darjeeling_actuators_Servo.c
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

// short javax.darjeeling.actuators.Servo.getNrServos()
void javax_darjeeling_actuators_Servo_short_getNrServos()
{
	dj_exec_stackPushShort(0);
}

// void javax.darjeeling.actuators.Servo.set(short, short)
void javax_darjeeling_actuators_Servo_void_set_short_short()
{
	dj_exec_stackPopShort();
	dj_exec_stackPopShort();

	// do nothing
}

// void javax.darjeeling.actuators.Servo.setPulseRange(short, short, short)
void javax_darjeeling_actuators_Servo_void_setPulseRange_short_short_short()
{
	dj_exec_stackPopShort();
	dj_exec_stackPopShort();
	dj_exec_stackPopShort();

	// do nothing
}
