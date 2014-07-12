/*
 * tossim_ugly.h
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
 
#ifndef __TOSSIM_UGLY_H__
#define __TOSSIM_UGLY_H__
//this header file is included in addition to tossim.h
//in order to resolve the cyclic dependancy between
//types.h, pointerwidth.h and tossim.h
//types.h includes pointerwidth.h which has a global variable (ref_t_base_address)
//so it wanted to include tossim.h for that, and tossim.h needed types.h
//in order to be able to define tossim_global_variables struct,
//Our solution in here is the most straightforward one,
//We will be more than happy to see a nicer solution,
#define _global_ref_t_base_address (getUglyGlobalVariables()->ref_t_base_address)

struct tossim_UGLY_global_variables{
	//------------------------------------------------
	//from darjeeling.c
	char *ref_t_base_address;
};
void* tossim_getDarjeelingUglyGlobals();
void tossim_setDarjeelingUglyGlobals(void *global_variables);

static inline struct tossim_UGLY_global_variables* getUglyGlobalVariables(){
	return tossim_getDarjeelingUglyGlobals();
}

static inline void setUglyGlobalVariables(struct tossim_UGLY_global_variables* _global_variables){
	tossim_setDarjeelingUglyGlobals((void *) _global_variables);
}


#endif
