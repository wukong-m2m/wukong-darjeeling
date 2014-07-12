/*
 * darjeeling.h
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
 
#ifndef __darjeeling_h__
#define __darjeeling_h__

#include <stdint.h>
#include "config.h"

void dj_init();
dj_time_t dj_run();

void dj_notifyRadioSendDone();
void dj_notifyRadioReceive();
void dj_notifySerialReceive();
void dj_notifyUartReceiveByte(uint8_t byte);

#endif
