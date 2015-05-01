/*
 * ElementId.java
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
 
package org.csiro.darjeeling.infuser.structure;

/**
 * Identifiers for the different Element types.
 *  
 * @author Niels Brouwers
 */
public enum ElementId
{
	
	HEADER(0),
	CLASSLIST(1),
	METHODIMPLLIST(2),
	STATICFIELDLIST(3),
	METHODDEFLIST(4),
	INFUSIONLIST(5),
	STRINGTABLE(6),
	
	CLASSDEF(7),
	METHODIMPL(8),
	INFUSION(9),
	METHODDEF(10),
	METHOD(11),
	FIELDDEF(12),
	;
	
	private int id;
	
	private ElementId(int id)
	{
		this.id = id;
	}
	
	/**
	 * @return an integer value that corresponds to an ElementID. 
	 */
	public int getId()
	{
		return id;
	}

}
