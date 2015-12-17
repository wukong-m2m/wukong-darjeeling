/*
 * UseSINC.java
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
import org.csiro.darjeeling.infuser.bytecode.instructions.*;
import org.csiro.darjeeling.infuser.bytecode.InstructionHandle;
import org.csiro.darjeeling.infuser.bytecode.InstructionList;
import org.csiro.darjeeling.infuser.bytecode.Opcode;
import org.csiro.darjeeling.infuser.bytecode.LocalVariable;
import org.csiro.darjeeling.infuser.bytecode.analysis.GeneratedValue;
import org.csiro.darjeeling.infuser.bytecode.analysis.GeneratedValueSet;
import org.csiro.darjeeling.infuser.bytecode.instructions.ExplicitCastInstruction;
import org.csiro.darjeeling.infuser.structure.BaseType;
import org.csiro.darjeeling.infuser.structure.TypeClass;

public class UseSINC extends CodeBlockTransformation
{

	public UseSINC(CodeBlock codeBlock)
	{
		super(codeBlock);
	}
	
	private boolean isSLOAD(InstructionHandle inst) {
		Opcode opcode = inst.getInstruction().getOpcode();
		return opcode == Opcode.SLOAD
			|| opcode == Opcode.SLOAD_0
			|| opcode == Opcode.SLOAD_1
			|| opcode == Opcode.SLOAD_2
			|| opcode == Opcode.SLOAD_3;
	}

	private boolean isSCONST(InstructionHandle inst) {
		return inst.getInstruction() instanceof ConstantPushInstruction;
	}

	private boolean isSADD(InstructionHandle inst) {
		Opcode opcode = inst.getInstruction().getOpcode();
		return opcode == Opcode.SADD;	
	}

	private boolean isSSTORE(InstructionHandle inst) {
		Opcode opcode = inst.getInstruction().getOpcode();
		return opcode == Opcode.SSTORE
			|| opcode == Opcode.SSTORE_0
			|| opcode == Opcode.SSTORE_1
			|| opcode == Opcode.SSTORE_2
			|| opcode == Opcode.SSTORE_3;
	}

	private LocalVariable getLocalVariable(InstructionHandle inst) {
		LoadStoreInstruction ls = (LoadStoreInstruction)inst.getInstruction();
		return ls.getLocalVariable();
	}

	private int getConstant(InstructionHandle inst) {
		ConstantPushInstruction cs = (ConstantPushInstruction)inst.getInstruction();
		return (int)cs.getValue();
	}

	@Override
	protected void transformInternal()
	{		
		InstructionList instructions = codeBlock.getInstructions();
		
		for (int i=0; i<instructions.size()-3; i++)
		{
			// For increment instructions like "x++;" where x is a short instead of an int, javac will produce something like this:
			// 0000;             ; 1           ; sload(0)                      ;                                                             ; 0Short                                                      ;   ; 1,0
			// 0001; 0           ; 2           ; sconst_1                      ; 0Short                                                      ; 0Short,1Short(Byte)                                         ;   ; 1
			// 0002; 1           ; 3           ; sadd                          ; 0Short,1Short(Byte)                                         ; 2Short                                                      ; G ; 1
			// 0003; 2           ; 4           ; sstore(0)                     ; 2Short                                                      ;                                                             ;   ; 1,0
			//
			// Darjeeling has a SINC instruction, but since all S- instructions are optimised from the I- original if the datatype allows, SINC
			// was never used because javac won't output a IINC for "x++;"
			// Instead we need to recognise this sequence of instructions and replace it with the appropriate SINC instruction.
			InstructionHandle handleLOAD = instructions.get(i);
			InstructionHandle handleCONST = instructions.get(i+1);
			InstructionHandle handleADD = instructions.get(i+2);
			InstructionHandle handleSTORE = instructions.get(i+3);

			if (isSLOAD(handleLOAD)
				&& isSCONST(handleCONST)
				&& isSADD(handleADD)
				&& isSSTORE(handleSTORE)
				&& getLocalVariable(handleLOAD) == getLocalVariable(handleSTORE)) {
			int increment = getConstant(handleCONST);
			LocalVariable variable = getLocalVariable(handleLOAD);

			InstructionHandle handleSINC = ((byte)increment)==increment
										 ? new InstructionHandle(new IncreaseInstruction(Opcode.SINC, variable, increment))
										 : new InstructionHandle(new WideIncreaseInstruction(Opcode.SINC_W, variable, increment));
			instructions.insertBefore(handleLOAD, handleSINC);
			instructions.remove(handleLOAD);
			instructions.remove(handleCONST);
			instructions.remove(handleADD);
			instructions.remove(handleSTORE);
			}
		}
	}
}
