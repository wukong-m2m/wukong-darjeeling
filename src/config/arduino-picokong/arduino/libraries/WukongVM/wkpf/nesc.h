/*
 * nesc.h
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
 
 
#ifndef __nesc_h__
#define __nesc_h__

#include "stdint.h"
#include "config.h"

int32_t nesc_getNodeId();

// Standard output
int nesc_printf(char * msg);

// Timer
dj_time_t nesc_getTime();

// Leds
void nesc_setLed(int nr, int on);

// Radio
uint16_t nesc_getMaxPayloadLength();
int nesc_send(const char * message, int16_t receiverId, uint16_t length);
int nesc_wasAcked();
uint16_t nesc_peekMessageLength();
void * nesc_popMessageBuffer();
int nesc_getNrMessages();

// Serial
void nesc_uartWriteByte(uint8_t byte);

#endif
