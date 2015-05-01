/*
 * AnalyseTypes.java
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
import org.csiro.darjeeling.infuser.bytecode.Instruction;
import org.csiro.darjeeling.infuser.bytecode.InstructionHandle;
import org.csiro.darjeeling.infuser.bytecode.InstructionList;
import org.csiro.darjeeling.infuser.bytecode.LocalVariable;
import org.csiro.darjeeling.infuser.bytecode.Opcode;
import org.csiro.darjeeling.infuser.bytecode.analysis.Interpreter;
import org.csiro.darjeeling.infuser.bytecode.instructions.LocalVariableInstruction;
import org.csiro.darjeeling.infuser.structure.BaseType;

public class AnalyseTypes extends CodeBlockTransformation
{

	public AnalyseTypes(CodeBlock codeBlock)
	{
		super(codeBlock);
	}

	@Override
	protected void transformInternal()
	{

		// create an interpreter and to type inference
		Interpreter interpreter = new Interpreter(codeBlock);
		interpreter.inferTypes();
		interpreter.inferLiveRanges();
		
		codeBlock.getInstructions().fixBranchAddresses();
		
		// determine the types of the local variables
		InstructionList instructions = codeBlock.getInstructions();
		for (int i=0; i<instructions.size(); i++)
		{
			InstructionHandle handle = instructions.get(i);
			
			Instruction instruction = handle.getInstruction();
			
			if (instruction.getOpcode().isStoreInstruction())
			{
				LocalVariable localVariable = ((LocalVariableInstruction)instruction).getLocalVariable();
				localVariable.setType(handle.getPreState().getStack().peek().getLogicalType());
			}
			
			if (instruction.getOpcode()==Opcode.IINC || instruction.getOpcode()==Opcode.IINC_W)
			{
				LocalVariable localVariable = ((LocalVariableInstruction)instruction).getLocalVariable();
				localVariable.setType(BaseType.Int);
			}
			
		}
		
	}

}
