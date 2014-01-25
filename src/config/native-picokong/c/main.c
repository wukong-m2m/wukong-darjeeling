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
#include <string.h>
#include <getopt.h>

#include "core.h"
#include "types.h"
#include "hooks.h"
#include "djarchive.h"
#include "wkpf_main.h"
#include "posix_utils.h"

int main(int argc,char* argv[])
{
	posix_parse_command_line(argc, argv);

	// Read the lib and app infusion archives from file
	di_app_archive = posix_load_infusion_archive(posix_app_infusion_filename);

	// initialise memory manager
	void *mem = malloc(HEAPSIZE);
	ref_t_base_address = (char*)mem - 42;

	core_init(mem, HEAPSIZE);
	dj_exec_setRunlevel(RUNLEVEL_RUNNING);
	wkpf_picokong(di_app_archive);

	return 0;
}
