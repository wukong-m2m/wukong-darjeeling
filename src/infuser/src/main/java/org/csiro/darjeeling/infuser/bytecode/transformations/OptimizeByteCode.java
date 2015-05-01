/*
 * OptimizeByteCode.java
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

import java.util.HashMap;

import org.csiro.darjeeling.infuser.bytecode.CodeBlock;
import org.csiro.darjeeling.infuser.bytecode.CodeBlockTransformation;
import org.csiro.darjeeling.infuser.bytecode.Instruction;
import org.csiro.darjeeling.infuser.bytecode.InstructionHandle;
import org.csiro.darjeeling.infuser.bytecode.InstructionList;
import org.csiro.darjeeling.infuser.bytecode.LocalVariable;
import org.csiro.darjeeling.infuser.bytecode.Opcode;
import org.csiro.darjeeling.infuser.bytecode.analysis.InterpreterStack;
import org.csiro.darjeeling.infuser.bytecode.instructions.AbstractInvokeInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.LocalVariableInstruction;
import org.csiro.darjeeling.infuser.structure.BaseType;
import org.csiro.darjeeling.infuser.structure.TypeClass;

public class OptimizeByteCode extends CodeBlockTransformation
{
	
	private static HashMap<Opcode, Opcode> optimisedOpcodes;

	static
	{
		optimisedOpcodes = new HashMap<Opcode, Opcode>();
		optimisedOpcodes.put(Opcode.ISTORE, Opcode.SSTORE);
		optimisedOpcodes.put(Opcode.ILOAD, Opcode.SLOAD);

		optimisedOpcodes.put(Opcode.IRETURN, Opcode.SRETURN);

		optimisedOpcodes.put(Opcode.BIPUSH, Opcode.BSPUSH);
		optimisedOpcodes.put(Opcode.SIPUSH, Opcode.SSPUSH);
		
		optimisedOpcodes.put(Opcode.ICONST_M1, Opcode.SCONST_M1);
		optimisedOpcodes.put(Opcode.ICONST_0, Opcode.SCONST_0);
		optimisedOpcodes.put(Opcode.ICONST_1, Opcode.SCONST_1);
		optimisedOpcodes.put(Opcode.ICONST_2, Opcode.SCONST_2);
		optimisedOpcodes.put(Opcode.ICONST_3, Opcode.SCONST_3);
		optimisedOpcodes.put(Opcode.ICONST_4, Opcode.SCONST_4);
		optimisedOpcodes.put(Opcode.ICONST_5, Opcode.SCONST_5);
		
		optimisedOpcodes.put(Opcode.IF_ICMPEQ, Opcode.IF_SCMPEQ);
		optimisedOpcodes.put(Opcode.IF_ICMPNE, Opcode.IF_SCMPNE);
		optimisedOpcodes.put(Opcode.IF_ICMPLT, Opcode.IF_SCMPLT);
		optimisedOpcodes.put(Opcode.IF_ICMPLE, Opcode.IF_SCMPLE);
		optimisedOpcodes.put(Opcode.IF_ICMPGT, Opcode.IF_SCMPGT);
		optimisedOpcodes.put(Opcode.IF_ICMPGE, Opcode.IF_SCMPGE);

		optimisedOpcodes.put(Opcode.IIFEQ, Opcode.SIFEQ);
		optimisedOpcodes.put(Opcode.IIFGE, Opcode.SIFGE);
		optimisedOpcodes.put(Opcode.IIFGT, Opcode.SIFGT);
		optimisedOpcodes.put(Opcode.IIFLE, Opcode.SIFLE);
		optimisedOpcodes.put(Opcode.IIFLT, Opcode.SIFLT);
		optimisedOpcodes.put(Opcode.IIFNE, Opcode.SIFNE);

		optimisedOpcodes.put(Opcode.IADD, Opcode.SADD);
		optimisedOpcodes.put(Opcode.ISUB, Opcode.SSUB);
		optimisedOpcodes.put(Opcode.IMUL, Opcode.SMUL);
		optimisedOpcodes.put(Opcode.IDIV, Opcode.SDIV);
		optimisedOpcodes.put(Opcode.IREM, Opcode.SREM);
		optimisedOpcodes.put(Opcode.INEG, Opcode.SNEG);
		optimisedOpcodes.put(Opcode.ISHL, Opcode.SSHL);
		optimisedOpcodes.put(Opcode.ISHR, Opcode.SSHR);
		optimisedOpcodes.put(Opcode.IUSHR, Opcode.SUSHR);
		optimisedOpcodes.put(Opcode.IAND, Opcode.SAND);
		optimisedOpcodes.put(Opcode.IOR, Opcode.SOR);
		optimisedOpcodes.put(Opcode.IXOR, Opcode.SXOR);
		
		optimisedOpcodes.put(Opcode.IINC, Opcode.SINC);
		optimisedOpcodes.put(Opcode.IINC_W, Opcode.SINC_W);	
		
	}

	public OptimizeByteCode(CodeBlock codeBlock)
	{
		super(codeBlock);
	}
	
	private void optimise()
	{
		InstructionList instructions =  codeBlock.getInstructions();
		BaseType operandTypes[];
		
		// backwards pass
		for (int i=instructions.size()-1; i>=0; i--)
		{
			InstructionHandle handle = instructions.get(i);
			Instruction instruction = handle.getInstruction();
			InterpreterStack stack = handle.getPreState().getStack();
			Opcode opcode = instruction.getOpcode();
			boolean forceOptimise = false;
			
			// Propagate the optimisation hints upwards
			handle.propagateOptimisationHints();
			handle.getInstruction().setOptimisationHints(handle);
				
			switch (opcode)
			{
				
				// invoke instructions
				case INVOKESTATIC:
				case INVOKEINTERFACE:
				case INVOKESPECIAL:
				case INVOKEVIRTUAL:
					operandTypes = ((AbstractInvokeInstruction)instruction).getMethodDefinition().getArgumentTypes();
					
					for (int j=0; j<operandTypes.length; j++)
						handle.getPreState().getStack().peek(j).setOptimizationHint(operandTypes[operandTypes.length-1-j]);
					break;
					
				case IADD:
				case ISUB:
				case IAND:
				case IOR:
				case IXOR:	
					if (handle.getOptimisationHint()==BaseType.Short) forceOptimise = true;
					
				case IMUL:
				case IDIV:
				case IREM:
				case INEG:
				case ISHL:
				case ISHR:
				case IUSHR:
					
					if (!forceOptimise)
					{
						if (stack.peek(0).getLogicalType().getTypeClass()!=TypeClass.Short) break;
						if (opcode!=Opcode.INEG && stack.peek(1).getLogicalType().getTypeClass()!=TypeClass.Short) break;
					}
					
					// never optimise if the requested output type of this instruction is not short, and the 
					// instruction generates overflow
					if (handle.getOptimisationHint()!=BaseType.Short && handle.generatesOverflow()) break;
					
					// never optimise if we need to preserve the overflow
					if (handle.generatesOverflow() && handle.isKeepOverflow()) break;
					
					// optimise the instruction
					if (optimisedOpcodes.containsKey(opcode))
					{
						instruction.setOpcode(optimisedOpcodes.get(opcode));
						handle.setOptimisationHint(0, BaseType.Short);
						if (opcode!=Opcode.INEG) handle.setOptimisationHint(1, BaseType.Short);
						
						// reset :3
						i=instructions.size()-1;
					}
					
					
					break;
				
				case ISTORE:
				case ILOAD:
				case LSTORE:
				case LLOAD:
				case IINC:
				case IINC_W:
					LocalVariable localVariable = ((LocalVariableInstruction)instruction).getLocalVariable();
					if (localVariable.getMaxIntType().getTypeClass()==TypeClass.Short && optimisedOpcodes.containsKey(opcode))
					{
						instruction.setOpcode(optimisedOpcodes.get(opcode));

						// set hint to 'short' for store instructions
						if (opcode==Opcode.ISTORE)
							handle.setOptimisationHint(0, BaseType.Short);
						
						// reset :3
						i=instructions.size()-1;
					} else
					{
						// set hint for store instructions
						if (opcode==Opcode.ISTORE) handle.setOptimisationHint(0, BaseType.Int);

					}
					break;
					
				case SIPUSH:
				case BIPUSH:
				case ICONST_M1:
				case ICONST_0:
				case ICONST_1:
				case ICONST_2:
				case ICONST_3:
				case ICONST_4:
				case ICONST_5:
					
					if (handle.getOptimisationHint().getTypeClass()==TypeClass.Short && optimisedOpcodes.containsKey(opcode))
					{
						instruction.setOpcode(optimisedOpcodes.get(opcode));

						// reset :3
						i=instructions.size()-1;
					}
					break;
					
				case IF_ICMPEQ:
				case IF_ICMPNE:
				case IF_ICMPGE:
				case IF_ICMPGT:
				case IF_ICMPLE:
				case IF_ICMPLT:
					// optimise if both operands are short typed
					if (handle.getOperandType(0).getTypeClass()==TypeClass.Short && handle.getOperandType(1).getTypeClass()==TypeClass.Short)
					{
						instruction.setOpcode(optimisedOpcodes.get(opcode));
						handle.setOptimisationHint(0, BaseType.Short);
						handle.setOptimisationHint(1, BaseType.Short);
					}	
					break;
					
				case IIFEQ:
				case IIFNE:
				case IIFGE:
				case IIFGT:
				case IIFLE:
				case IIFLT:
					// optimise if operand is short typed
					if (handle.getOperandType(0).getTypeClass()==TypeClass.Short)
					{
						instruction.setOpcode(optimisedOpcodes.get(opcode));
						handle.setOptimisationHint(0, BaseType.Short);
					}	
					break;
					
				case IRETURN:
					BaseType returnType = codeBlock.getMethodImplementation().getMethodDefinition().getReturnType();
					if (returnType.getTypeClass()==TypeClass.Short)
					{
						instruction.setOpcode(optimisedOpcodes.get(opcode));
						handle.setOptimisationHint(0, BaseType.Short);
					}
					break;
				
				// narrowing operator
				case I2B:
					handle.getInstruction().setOpcode(Opcode.S2B);
					handle.setOptimisationHint(0, BaseType.Short);

					// reset :3
					i=instructions.size()-1;
					break;
					
				case I2C:
					handle.getInstruction().setOpcode(Opcode.S2C);
					handle.setOptimisationHint(0, BaseType.Short);

					// reset :3
					i=instructions.size()-1;
					break;

				case I2S:
					// S2S is a temporary instruction, will be pruned later
					handle.getInstruction().setOpcode(Opcode.S2S);
					handle.setOptimisationHint(0, BaseType.Short);
					
					// reset :3
					i=instructions.size()-1;
					break;

			}
			
		}
	}
	
	@Override
	protected void transformInternal()
	{
		optimise();
	}

}
