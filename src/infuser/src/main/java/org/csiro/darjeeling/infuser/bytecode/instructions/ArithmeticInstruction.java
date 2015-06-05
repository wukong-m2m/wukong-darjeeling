/*
 * ArithmeticInstruction.java
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
import org.csiro.darjeeling.infuser.bytecode.analysis.InterpreterState;
import org.csiro.darjeeling.infuser.structure.BaseType;

public class ArithmeticInstruction extends SimpleInstruction
{
	
	public ArithmeticInstruction(Opcode opcode)
	{
		super(opcode);
	}
	
	private void setKeepOverflowFlag(InstructionHandle handle)
	{
		InterpreterState preState = handle.getPreState();
		
		switch (opcode)
		{
			case IREM:
			case IDIV:
				preState.getStack().peek(0).setKeepOverflow(true);
				preState.getStack().peek(1).setKeepOverflow(true);
				break;

			case INEG:
				preState.getStack().peek(0).setKeepOverflow(true);
				break;
				
			case ISHR:
			case IUSHR:
				preState.getStack().peek(1).setKeepOverflow(true);
				break;
				
			case IIFEQ:
			case IIFGE:
			case IIFGT:
			case IIFLE:
			case IIFLT:
			case IIFNE:
				preState.getStack().peek(0).setKeepOverflow(true);
				break;
				
			case IF_ICMPEQ:
			case IF_ICMPGE:
			case IF_ICMPGT:
			case IF_ICMPLT:
			case IF_ICMPLE:
			case IF_ICMPNE:
				preState.getStack().peek(0).setKeepOverflow(true);
				preState.getStack().peek(1).setKeepOverflow(true);
				break;
				
			case LOOKUPSWITCH:
			case TABLESWITCH:
				preState.getStack().peek(0).setKeepOverflow(true);
				break;

			case NEWARRAY:
				preState.getStack().peek(0).setKeepOverflow(true);
				break;

			case BALOAD:
			case CALOAD:
			case SALOAD:
			case IALOAD:
			case AALOAD:
				preState.getStack().peek(0).setKeepOverflow(true);
				break;
				
			case BASTORE:
			case CASTORE:
			case SASTORE:
			case IASTORE:
			case AASTORE:
				preState.getStack().peek(1).setKeepOverflow(true);
				break;
				
		}
			
	}
	
	private void setCausesOverflowFlag(InstructionHandle handle)	
	{
		BaseType a,b;
		Opcode opcode = handle.getInstruction().getOpcode();
		InterpreterState preState = handle.getPreState();

		switch (opcode)
		{
			case IADD:
			case ISUB:
			case IMUL:
				a = preState.getStack().peek(1).getLogicalType();
				b = preState.getStack().peek(0).getLogicalType();

				// if one of the types is int, or if both types are byte-sized (byte, boolean, char) 
				// this instruction does not carry potential overflow
				if (a.isIntSized() || b.isIntSized()) break;		// op(int, *), op(*, int)
				if (a.isByteSized() && b.isByteSized()) break;		// op(byte, byte)
				
				// else this instruction may cause overflow
				handle.setGeneratesOverflow(true);
				break;

			case IDIV:
				a = preState.getStack().peek(1).getLogicalType();
				b = preState.getStack().peek(0).getLogicalType();

				// if one of the types is int, or if both types are byte-sized (byte, boolean, char) 
				// this instruction does not carry potential overflow
				if (a.isIntSized() || b.isIntSized()) break;		// int
				if (a.isByteSized() && b.isShortSized()) break;		// byte/short
				if (a.isByteSized() && b.isByteSized()) break;		// byte/byte
				
				// else this instruction may cause overflow
				handle.setGeneratesOverflow(true);
				break;

			case ISHL:
				a = preState.getStack().peek(1).getLogicalType();

				// This instruction may cause overflow if the operand type is short
				handle.setGeneratesOverflow(a!=BaseType.Int);
				break;
				
			case INEG:
				a = preState.getStack().peek(0).getLogicalType();

				// This instruction may cause overflow if the operand type is short
				handle.setGeneratesOverflow(a.isShortSized());
				break;

		}
			
	}
	
	
	@Override
	public void setOptimisationHints(InstructionHandle handle)
	{
		super.setOptimisationHints(handle);
		setKeepOverflowFlag(handle);
		setCausesOverflowFlag(handle);
	}

}
