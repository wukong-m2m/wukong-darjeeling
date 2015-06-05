/*
 * LocalVariable.java
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
 
package org.csiro.darjeeling.infuser.bytecode;

import java.util.Collection;
import java.util.HashSet;

import org.csiro.darjeeling.infuser.structure.BaseType;

/**
 * Describes a single local variable in a code block. The object is created with a 'slot number' as parameter. Initially, when reading
 * raw Java bytecode as input, one LocalVariable object is created for each local variable slot. Type inference is used to determine
 * how each slot is used. A variable slot may be used to store both integer and reference variables. During type inference a set of 
 * possible types is constructed. If at some point in the code for instance, an ASTORE instruction puts a reference variable into a 
 * slot, BaseType.Ref is added to the set of types.  
 * <p>
 * Because Darjeeling has two sets of local variables, for reference and non-reference (integer) types, each LocalVariable has to be mapped
 * onto a slot in either or both of these sets. This is where the <i>referenceIndex</i> and <i>integerIndex</i> fields come into play.
 * For both fields a value of -1 indicates that the local variable is not mapped into its respective set. Any other value indicates the
 * slot index.     
 * 
 * @author Niels Brouwers
 */
public class LocalVariable
{

	// The index of a local variable is the slot number in which it is stored. Directly after a method's byte code is read
	// this index holds the original index. 
	private int slot;
	
	// After local variables are re-mapped these indices hold the slot numbers of this local variable in the reference and integer
	// local variable blocks respectively
	private int referenceIndex, integerIndex;
	
	// This array holds all the types that are stored in the local variable by the function at different points in the code.
	// Some optimizing compiles may use a local variable slot for holding a reference type at some point, and as an integer 
	// variable later on. This set gathers all the uses of this slot into a set.
	// 
	// The type inference mechanism fills this array by looking at ASTORE/ALOAD and ISTORE/ILOAD instructions and
	// by looking at the parameter list of a method. 
	private HashSet<BaseType> types;
	
	private boolean isParameter;
	
	public LocalVariable(int slot)
	{
		types = new HashSet<BaseType>();
		this.slot = slot;
		referenceIndex = integerIndex = -1;
	}
	
	public BaseType getMaxIntType()
	{
		if (types.contains(BaseType.Long)) return BaseType.Long; 
		if (types.contains(BaseType.Int)) return BaseType.Int; 
		if (types.contains(BaseType.Short)) return BaseType.Short; 
		if (types.contains(BaseType.Boolean)) return BaseType.Boolean; 
		if (types.contains(BaseType.Char)) return BaseType.Char;
		if (types.contains(BaseType.Byte)) return BaseType.Byte; 
		
		return BaseType.Unknown;
	}
	
	public int getSlot()
	{
		return slot;
	}
	
	public int getReferenceIndex()
	{
		return referenceIndex;
	}
	
	public int getIntegerIndex()
	{
		return integerIndex;
	}
	
	public void setReferenceIndex(int referenceIndex)
	{
		this.referenceIndex = referenceIndex;
	}
	
	public void setIntegerIndex(int integerIndex)
	{
		this.integerIndex = integerIndex;
	}
	
	public void setType(BaseType type)
	{
		types.add(type);
	}
	
	public Collection<BaseType> getTypes()
	{
		return types;
	}
	
	public void setIsParameter(boolean isParameter)
	{
		this.isParameter = isParameter;
	}
	
	public boolean isParameter()
	{
		return isParameter;
	}
	
	@Override
	public String toString()
	{
		return String.format("%d: (%d:%d) %s %s", slot, referenceIndex, integerIndex, types.toString(), isParameter?"(parameter)":"");
	}
	
}
