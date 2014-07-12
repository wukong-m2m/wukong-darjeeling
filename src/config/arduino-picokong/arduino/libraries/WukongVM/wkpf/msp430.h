/*
 * msp430.h
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
 
#ifndef __msp430_h__
#define __msp430_h__

#include <stdio.h>
#include <stdarg.h>

// TODONR: copied from arduino platform. need to port to msp430.

// // 16 MHz clock speed
// #define F_CPU 16000000

// // clear bit, set bit macros
// #ifndef cbi
// #define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
// #endif
// #ifndef sbi
// #define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
// #endif

void msp430_timerInit();
unsigned long msp430_millis();
void msp430_delay(unsigned long ms);

void msp430_serialInit(uint32_t baud);
void msp430_serialPrint(char * str);
void msp430_serialVPrint(char * format, va_list arg);
void msp430_serialPrintf(char * format, ...);
void msp430_serialWrite(unsigned char value);


#endif
