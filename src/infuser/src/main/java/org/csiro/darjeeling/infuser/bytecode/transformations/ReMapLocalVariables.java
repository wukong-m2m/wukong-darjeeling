/*
 * ReMapLocalVariables.java
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
 
package org.csiro.darjeeling.infuser.bytecode.transformations;

import java.util.ArrayList;

import org.csiro.darjeeling.infuser.bytecode.CodeBlock;
import org.csiro.darjeeling.infuser.bytecode.CodeBlockTransformation;
import org.csiro.darjeeling.infuser.bytecode.InstructionHandle;
import org.csiro.darjeeling.infuser.bytecode.LocalVariable;
import org.csiro.darjeeling.infuser.structure.BaseType;

public class ReMapLocalVariables extends CodeBlockTransformation
{
	
	/*
	 * Private class that represents a single local variable slot. 
	 */
	@SuppressWarnings("serial")
	private class Slot extends ArrayList<LocalVariable>
	{
		public boolean overlaps(LocalVariable a, LocalVariable b)
		{
			for (InstructionHandle handle : codeBlock.getInstructions().getInstructionHandles())
				if (handle.getLiveVariables().contains(a) && handle.getLiveVariables().contains(b))
					return true;
			
			return false;
		}		
		
		/**
		 * @return true iff the local variable overlaps with this local variable slot.
		 */
		public boolean overlaps(LocalVariable localVariable)
		{
			for (LocalVariable slotVariable : this)
				if (slotVariable.isParameter() || overlaps(localVariable, slotVariable))
					return true;
			
			return false;
		}
	}
	
	@SuppressWarnings("serial")
	private class Mapping extends ArrayList<Slot>
	{
		public Slot get(int index)
		{
			while (size()<=index) this.add(new Slot());
			return super.get(index);
		}
		
		public boolean fitsAt(int index, int numSlots, LocalVariable localVariable)
		{
			for (int i=0; i<numSlots; i++)
			{
				Slot slot = this.get(i+index);
				if (slot.overlaps(localVariable))
					return false;
			}
			
			return true;
		}
		
		public int insert(int numSlots, LocalVariable localVariable)
		{
			int index;
			
			for (index=0; !fitsAt(index, numSlots, localVariable); index++);
			
			for (int i=0; i<numSlots; i++)
			{
				Slot slot = this.get(i+index);
				slot.add(localVariable);
			}
			
			return index;
		}	
		
		public int add(int numSlots, LocalVariable localVariable)
		{
			int ret = this.size();
			
			for (int i=0; i<numSlots; i++)
			{
				Slot newSlot = new Slot();
				newSlot.add(localVariable);
				this.add(newSlot);
			}
			
			return ret;
		}
		
	}
	
	private Mapping integerMapping, referenceMapping;
	
	public ReMapLocalVariables(CodeBlock codeBlock)
	{
		super(codeBlock);
		integerMapping = new Mapping();		
		referenceMapping = new Mapping();
	}
	
	@Override
	public void transformInternal()
	{
		
		// add the parameter variables first
		for (int i=0; i<codeBlock.getLocalVariableCount(); i++)
		{
			LocalVariable local = codeBlock.getLocalVariable(i);
			BaseType maxInt = local.getMaxIntType();
			if (local.isParameter())
			{
				if (local.getTypes().contains(BaseType.Ref))
					local.setReferenceIndex(referenceMapping.add(1, local));
				
				if (maxInt!=BaseType.Unknown) 
					local.setIntegerIndex(integerMapping.add(maxInt.getNrIntegerSlots(), local));
			}
		}
		
		// insert the other parameters next (these may not overlap with parameter variables)
		for (int i=0; i<codeBlock.getLocalVariableCount(); i++)
		{
			LocalVariable local = codeBlock.getLocalVariable(i);
			BaseType maxInt = local.getMaxIntType();
			
			if (!local.isParameter())
			{
				if (local.getTypes().contains(BaseType.Ref))
					local.setReferenceIndex(referenceMapping.insert(1, local));
				
				if (maxInt!=BaseType.Unknown) 
					local.setIntegerIndex(integerMapping.insert(maxInt.getNrIntegerSlots(), local));
			}
				
		}
		
		codeBlock.setIntegerLocalVariableCount(integerMapping.size());
		codeBlock.setReferenceLocalVariableCount(referenceMapping.size());
	}

}
