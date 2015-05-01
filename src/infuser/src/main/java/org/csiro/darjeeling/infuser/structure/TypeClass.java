/*
 * TypeClass.java
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

public enum TypeClass
{
	
	Short(1,0),
	Int(2,0),
	Long(4,0),
	Reference(0,1),
	Void(0,0);
	
	private int nrIntegerSlots, nrReferenceSlots;
	
	private TypeClass(int nrIntegerSlots, int nrReferenceSlots)
	{
		this.nrIntegerSlots = nrIntegerSlots;
		this.nrReferenceSlots = nrReferenceSlots;
	}
	
	public int getNrIntegerSlots()
	{
		return nrIntegerSlots;
	}
	
	public int getNrReferenceSlots()
	{
		return nrReferenceSlots;
	}
	
}
