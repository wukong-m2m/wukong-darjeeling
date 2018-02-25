/*
 * GeneratedValue.java
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
 
package org.csiro.darjeeling.infuser.bytecode.analysis;

import org.csiro.darjeeling.infuser.bytecode.InstructionHandle;
import org.csiro.darjeeling.infuser.bytecode.Opcode;
import org.csiro.darjeeling.infuser.bytecode.instructions.LocalIdInstruction;
import org.csiro.darjeeling.infuser.structure.BaseType;
import org.csiro.darjeeling.infuser.structure.ConstArrayHandler;
import org.csiro.darjeeling.infuser.structure.LocalId;

public class GeneratedValue implements Comparable<GeneratedValue>
{

	private InstructionHandle handle;
	private int index;
	private BaseType optimizationHint;
	
	public GeneratedValue(InstructionHandle handle, int index)
	{
		this.handle = handle;
		this.index = index;
	}
	
	public BaseType getOutputType()
	{
		return handle.getOutputType(index);
	}
	
	public BaseType getLogicalOutputType()
	{
		return handle.getLogicalOutputType(index);
	}
	
	public InstructionHandle getHandle()
	{
		return handle;
	}
	
	public void setHandle(InstructionHandle handle)
	{
		this.handle = handle;
	}
	
	public int getIndex()
	{
		return index;
	}
	
	public void setIndex(int index)
	{
		this.index = index;
	}
	
	public BaseType getOptimizationHint()
	{
		return optimizationHint;
	}
	
	public void setOptimizationHint(BaseType optimizationHint)
	{
		this.optimizationHint = optimizationHint;
	}
	
	public int compareTo(GeneratedValue o)
	{
		if (o.handle!=handle) return 1;
		if (o.index!=index) return o.index>index?1:-1;
		return 0;
	}

	public boolean isConstArrayGETSTATICValue() {
		if (this.handle.getInstruction().getOpcode() == Opcode.GETSTATIC_A) {
			LocalId localId = ((LocalIdInstruction)handle.getInstruction()).getLocalId();
			if (localId.getInfusionId() == 0
					&& ConstArrayHandler.isStaticFieldIdConstArray(localId.getLocalId())) {
				return true;
			}
		}
		return false;
	}

	public int getConstArrayFieldId() {
		LocalId localId = ((LocalIdInstruction)handle.getInstruction()).getLocalId();
		return ConstArrayHandler.getConstArrayStringTableId(localId.getLocalId());
	}
}
