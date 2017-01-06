/*
 * main.c
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
 

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "jlib_base.h"
#include "jlib_darjeeling.h"

#include "types.h"
#include "vm.h"
#include "heap.h"
#include "execution.h"

#include "pointerwidth.h"
char * ref_t_base_address;

extern char * * _binary_infusions_start;
extern char * * _binary_infusions_end;
extern size_t * _binary_infusions_size;

int main(int argc,char* argv[])
{
	
}
