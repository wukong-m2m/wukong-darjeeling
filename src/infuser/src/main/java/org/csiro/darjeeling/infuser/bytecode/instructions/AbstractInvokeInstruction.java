/*
 * AbstractInvokeInstruction.java
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
 
package org.csiro.darjeeling.infuser.bytecode.instructions;

import org.csiro.darjeeling.infuser.bytecode.InstructionHandle;
import org.csiro.darjeeling.infuser.bytecode.Opcode;
import org.csiro.darjeeling.infuser.structure.BaseType;
import org.csiro.darjeeling.infuser.structure.LocalId;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodDefinition;

public abstract class AbstractInvokeInstruction extends LocalIdInstruction
{

	protected AbstractMethodDefinition methodDefinition;
	
	public AbstractInvokeInstruction(Opcode opcode, LocalId localId, AbstractMethodDefinition methodDefinition)
	{
		super(opcode, localId);
		this.methodDefinition = methodDefinition;		
	}
	
	public AbstractMethodDefinition getMethodDefinition()
	{
		return methodDefinition;
	}
	
	public BaseType[] getOperandTypes()
	{
		return methodDefinition.getArgumentTypes();
	}
	
	public void setOptimisationHints(InstructionHandle handle)
	{
		// set input hints
		BaseType operandTypes[] = methodDefinition.getArgumentTypes();
		
		for (int i=0; i<operandTypes.length; i++)
			handle.getPreState().getStack().peek(i).setOptimizationHint(operandTypes[operandTypes.length-1-i]);
	}
	
	@Override
	public int getNrOutputValues()
	{
		return methodDefinition.getReturnType()!=BaseType.Void?1:0;
	}
	
	@Override
	public BaseType getOuputType(int index, InstructionHandle handle)
	{
		if (index!=0) throw new IllegalArgumentException("invoke instructions produce at most one output");
		return methodDefinition.getReturnType();
	}
	
	public String toString()
	{
		return super.toString() + methodDefinition.getGlobalId() + "[" + methodDefinition.getName() + " " + methodDefinition.getSignature() + "]";
	}

}
