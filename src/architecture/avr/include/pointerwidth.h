/*
 * pointerwidth.h
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
 
#ifndef __pointerwidth_h
#define __pointerwidth_h

typedef void* ref_t;
#define nullref ((ref_t)0)

#define REF_TO_VOIDP(ref)  (ref)
#define VOIDP_TO_REF(ref)  (ref)

#define REF_TO_UINT32(ref) ((uint32_t)((uint16_t)ref))
#define UINT32_TO_REF(ref) ((void *)((uint16_t)ref))


#endif // __pointerwidth_h
