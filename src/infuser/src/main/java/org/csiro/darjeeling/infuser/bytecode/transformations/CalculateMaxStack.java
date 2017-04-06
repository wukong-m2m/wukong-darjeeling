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
import org.csiro.darjeeling.infuser.bytecode.analysis.GeneratedValue;
import org.csiro.darjeeling.infuser.bytecode.analysis.InterpreterStack;
import org.csiro.darjeeling.infuser.bytecode.analysis.InterpreterState;
import org.csiro.darjeeling.infuser.bytecode.instructions.StaticInvokeInstruction;
import org.csiro.darjeeling.infuser.structure.BaseType;
import org.csiro.darjeeling.infuser.structure.lightweightmethods.LightweightMethod;

public class CalculateMaxStack extends CodeBlockTransformation
{
	
	public CalculateMaxStack(CodeBlock codeBlock)
	{
		super(codeBlock);
	}

	@Override
	protected void transformInternal()
	{
		int maxStack = 0;
		int maxRefStack = 0;

		for (InstructionHandle handle : codeBlock.getInstructions().getInstructionHandles())
		{
			
			InterpreterState state = handle.getPreState();
			InterpreterStack stack = state.getStack();
			int stackSize = 0, refStackSize = 0;
			for (int i=0; i<stack.size(); i++)
			{
				BaseType type = stack.peek(i).getType();
				if (type==null)
				{
					// probably dummy ATHROW handle for exception handlers
					for (GeneratedValue pair: stack.peek(i))
						if (pair.getHandle().getInstruction().getOpcode()==Opcode.ATHROW)
							type = BaseType.Ref;
					
					// not a dummy ATHROW? Panic
					if (type==null)
						throw new IllegalStateException("Type inference error");
				}
				
				stackSize += type.getNrIntegerSlots() + type.getNrReferenceSlots();
				refStackSize += type.getNrReferenceSlots();
			}

			if (handle.getInstruction().getOpcode() == Opcode.INVOKELIGHT) {
				// Lightweight methods may use more stack space when they execute.
				// We need to reserve space for this
				// The parameters and return value are already handled by the code above.
				// If (max size of the called method - parameters) > 0 then we need to add the difference to stacksize (and similarly for ref stack)
				LightweightMethod l = ((StaticInvokeInstruction)handle.getInstruction()).getLightweightMethod();

				int extraIntSlots = l.getMaxIntStack() - l.getParameterIntStack();
				int extraRefSlots = l.getMaxRefStack() - l.getParameterRefStack();
				int extraSlots = extraIntSlots + extraRefSlots;
				if (extraRefSlots > 0) {
					refStackSize += extraRefSlots;
				}
				if (extraSlots > 0) {
					stackSize += extraSlots;
				}
			}

			maxStack = stackSize>maxStack?stackSize:maxStack;
			maxRefStack = refStackSize>maxRefStack?refStackSize:maxRefStack;
		}
		codeBlock.setMaxStack(maxStack+1);
		codeBlock.setMaxRefStack(maxRefStack+1); // Why +1? Don't know, but if it ever changes we should remove the -1 from LightweightMethod.determineMaxStackDepth to avoid reserving too little stack space!
	}

}
