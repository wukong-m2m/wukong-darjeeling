/*
 * spi.h
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
 
#ifndef __spi_h__
#define __spi_h__

#define avr_sspiSlaveSelect(x)		PORTB = (x) ? (PORTB|(1<<PB4)) : (PORTB&~(1<<PB4))
#define avr_sspiMoSi(x)				PORTB = (x) ? (PORTB|(1<<PB5)) : (PORTB&~(1<<PB5))
#define avr_sspiMiSo(x)				PORTB = (x) ? (PORTB|(1<<PB6)) : (PORTB&~(1<<PB6))
#define avr_sspiClock(x)			PORTB = (x) ? (PORTB|(1<<PB7)) : (PORTB&~(1<<PB7))


#define avr_sspiSlaveSelectEnable()		PORTB &= ~(1<<PB4)
#define avr_sspiSlaveSelectDisable()	PORTB |= (1<<PB4)

#define avr_sspiClockPulse()		PORTB &= ~(1<<PB7); \
									PORTB |= 1<<PB7;

#define avr_sspiSendBit(x)			avr_sspiMoSi(x); \
									avr_sspiClockPulse(); \

#define avr_sspiSendByte(x)			avr_sspiSendBit((x & 0x80) >> 7); \
									avr_sspiSendBit((x & 0x40) >> 6); \
									avr_sspiSendBit((x & 0x20) >> 5); \
									avr_sspiSendBit((x & 0x10) >> 4); \
									avr_sspiSendBit((x & 0x8) >> 3); \
									avr_sspiSendBit((x & 0x4) >> 2); \
									avr_sspiSendBit((x & 0x2) >> 1); \
									avr_sspiSendBit(x & 0x1);

#define avr_sspiInit()				DDRB |= 0xf0;

#endif
