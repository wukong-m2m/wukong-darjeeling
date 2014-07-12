/*
 * avr.h
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
 
#ifndef __avr_h__
#define __avr_h__

#include <avr/io.h>
#include <avr/interrupt.h>
// #include <avr/delay.h>
#include <stdio.h>
#include <stdarg.h>

// 16 MHz clock speed
#define F_CPU 16000000

// clear bit, set bit macros
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

void avr_timerInit();
unsigned long avr_millis();
void avr_delay(unsigned long ms);

void avr_serialInit(uint32_t baud);
void avr_serialPrint(char * str);
void avr_serialVPrint(char * format, va_list arg);
void avr_serialPrintf(char * format, ...);
void avr_serialWrite(unsigned char value);


#endif
