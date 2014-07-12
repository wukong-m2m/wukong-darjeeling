/*
 * dj_tos_message.h
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
 
 
#ifndef __dj_tos_message__
#define __dj_tos_message__

#include "tosconfig.h"

typedef nx_struct tos_message
{
  nx_uint8_t payload[TOS_RADIO_PAYLOADSIZE];
} tos_message_t;

enum {
	  DJ_TOS_MESSAGE = 0x11,
};

#endif
