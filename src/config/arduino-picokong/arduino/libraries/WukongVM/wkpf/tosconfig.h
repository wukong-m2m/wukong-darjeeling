/*
 * tosconfig.h
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
 
#define TOS_RADIO
#define TOS_SERIAL
// #define TOS_CC1000

// Uncomment to remove support for leds (may be useful for some low-power testing).
#define TOS_LEDS

// Size of a Darjeeling radio packet.
#define TOS_RADIO_PAYLOADSIZE 24

#define TOS_CC1101

// Size of the UART byte buffer.
#define SERIAL_BUFFERSIZE 8
