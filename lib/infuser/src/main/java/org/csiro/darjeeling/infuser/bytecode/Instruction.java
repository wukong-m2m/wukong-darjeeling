/*
 * Instruction.java
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

import java.io.DataOutputStream;
import java.io.IOException;

import org.csiro.darjeeling.infuser.structure.BaseType;

public abstract class Instruction
{
	
	protected Opcode opcode;
	
	protected Instruction(Opcode opcode)
	{
		if (opcode==null) throw new IllegalArgumentException("Opcode argument cannot be null");
		this.opcode = opcode;
	}
	
	public Opcode getOpcode()
	{
		return opcode;
	}
	
	public void setOpcode(Opcode opcode)
	{
		if (opcode==null) throw new IllegalArgumentException("Opcode argument cannot be null");
		this.opcode = opcode;
	}
	
	public abstract void dump(DataOutputStream out) throws IOException;
	public abstract int getLength();
	
	public BaseType[] getOperandTypes()
	{
		return opcode.getOperandTypes();
	}
	
	public void setOptimisationHints(InstructionHandle handle)
	{
		// set optimization hints
		BaseType operandTypes[] = opcode.getOperandTypes();
		for (int i=0; i<operandTypes.length; i++)
			handle.setOptimisationHint(operandTypes.length-1-i, operandTypes[i]);
		
	}
	
	public int getNrOutputValues()
	{
		return opcode.getOutputType()==null?0:1;
	}
	
	public int getNrDependentOperands()
	{
		return opcode.getOperandTypes().length;
	}
	
	public BaseType getOuputType(int index, InstructionHandle handle)
	{
		return opcode.getOutputType();
	}
	
	public BaseType getLogicalOutputType(int index, InstructionHandle handle)
	{
		return getOuputType(index, handle);
	}

	@Override
	public String toString()
	{
		return opcode.getName();
	}

}
