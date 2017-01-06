/*
 * printf.c
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
 
#include "stdio.h"
#include "string.h"

int nesc_printf(char*);

void g301_serialVPrint(char * format, va_list arg)
{
	char temp[128];
	vsnprintf(temp, 128, format, arg);
	nesc_printf(temp);
}

void g301_serialPrintf(char * format, ...)
{
	va_list arg;

	va_start(arg, format);
	g301_serialVPrint(format, arg);
	va_end(arg);

}

