/*
 * djtimer.c
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
 
#include <stddef.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "djtimer.h"
#include "types.h"

#include "avr.h"

void dj_timer_init()
{
}

dj_time_t dj_timer_getTimeMillis()
{
	return avr_millis();
}

void dj_timer_delay(unsigned long msec)
{
	return avr_delay(msec);
}

