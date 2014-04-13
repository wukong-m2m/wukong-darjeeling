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

typedef nx_struct radio_count_msg {
	  nx_uint8_t payload[24];
	  nx_uint8_t length;
//  nx_uint16_t counter;
} radio_count_msg_t;

/*
enum {
  AM_RADIO_COUNT_MSG = 6,
};
*/

/*

typedef nx_struct tos_message
{
  nx_uint8_t payload[24];
} tos_message_t;

enum {
	  DJ_TOS_MESSAGE = 0xf1,
};
*/

#endif
