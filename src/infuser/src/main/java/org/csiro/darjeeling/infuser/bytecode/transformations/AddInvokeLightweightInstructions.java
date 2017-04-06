/*
 * CalculateMaxStack.java
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

import org.csiro.darjeeling.infuser.bytecode.CodeBlock;
import org.csiro.darjeeling.infuser.bytecode.CodeBlockTransformation;
import org.csiro.darjeeling.infuser.bytecode.InstructionHandle;
import org.csiro.darjeeling.infuser.bytecode.Opcode;
import org.csiro.darjeeling.infuser.bytecode.Instruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.StaticInvokeInstruction;
import org.csiro.darjeeling.infuser.structure.lightweightmethods.LightweightMethod;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodImplementation;

public class AddInvokeLightweightInstructions extends CodeBlockTransformation
{
	
	public AddInvokeLightweightInstructions(CodeBlock codeBlock)
	{
		super(codeBlock);
	}

	@Override
	protected void transformInternal()
	{
		for (InstructionHandle handle : codeBlock.getInstructions().getInstructionHandles())
		{
			Instruction instruction = handle.getInstruction();
			if (instruction.getOpcode() == Opcode.INVOKESTATIC) {
				// Lightweight calls are always static
				StaticInvokeInstruction invoke = (StaticInvokeInstruction)instruction;
				AbstractMethodImplementation methodImplementation = invoke.getMethodImplementation();

				String className = methodImplementation.getParentClass().getName();
				String methodName = methodImplementation.getMethodDefinition().getName();

				if (LightweightMethod.isLightweightMethod(className, methodName)) {
					instruction.setOpcode(Opcode.INVOKELIGHT);
				}
			}
		}
	}
}
