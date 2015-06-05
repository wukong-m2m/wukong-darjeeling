/*
 * ReplaceStackInstructions.java
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
import java.util.Collection;

import org.csiro.darjeeling.infuser.bytecode.CodeBlock;
import org.csiro.darjeeling.infuser.bytecode.CodeBlockTransformation;
import org.csiro.darjeeling.infuser.bytecode.InstructionHandle;
import org.csiro.darjeeling.infuser.bytecode.InstructionList;
import org.csiro.darjeeling.infuser.bytecode.Opcode;
import org.csiro.darjeeling.infuser.bytecode.analysis.InterpreterState;
import org.csiro.darjeeling.infuser.bytecode.instructions.StackInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.WideStackInstruction;
import org.csiro.darjeeling.infuser.structure.BaseType;

public class ReplaceStackInstructions extends CodeBlockTransformation
{

	public ReplaceStackInstructions(CodeBlock codeBlock)
	{
		super(codeBlock);
	}
	
	private static int getNrIntegerSlots(BaseType ... types)
	{
		int ret = 0;
		for (BaseType type : types) ret += type.getNrIntegerSlots();
		return ret;
	}

	private static int getNrReferenceSlots(BaseType ... types)
	{
		int ret = 0;
		for (BaseType type : types) ret += type.getNrReferenceSlots();
		return ret;
	}

	private Collection<InstructionHandle> getIDupInstructions(int m, int n)
	{
		ArrayList<InstructionHandle> ret = new ArrayList<InstructionHandle>();
		
		int nn = (n==0)?0:(n+m);
		
		switch (n)
		{
			case 0:
				// shorthand opcodes for common case
				if (m==1) ret.add(new InstructionHandle(new StackInstruction(Opcode.IDUP)));
				if (m==2) ret.add(new InstructionHandle(new StackInstruction(Opcode.IDUP2)));
				// general case
				if (m>=3) ret.add(new InstructionHandle(new WideStackInstruction(Opcode.IDUP_X, m, nn)));
				break;
			case 1:
				// shorthand opcode for common case IDUP_X1
				if (m==1) ret.add(new InstructionHandle(new StackInstruction(Opcode.IDUP_X1)));
				// general case
				if (m>=2) ret.add(new InstructionHandle(new WideStackInstruction(Opcode.IDUP_X, m, nn)));
				break;
			case 2:
				// shorthand opcode for common case IDUP_X2
				if (m==1) ret.add(new InstructionHandle(new StackInstruction(Opcode.IDUP_X2)));
				if (m>=2) ret.add(new InstructionHandle(new WideStackInstruction(Opcode.IDUP_X, m, nn)));
				break;
			default:
				ret.add(new InstructionHandle(new WideStackInstruction(Opcode.IDUP_X, m, nn)));
				break;
		}

		return ret;
	}

	private Collection<InstructionHandle> getADupInstructions(int m, int n)
	{
		ArrayList<InstructionHandle> ret = new ArrayList<InstructionHandle>();

		switch (n)
		{
			case 0:
				// shorthand opcodes for common case
				if (m==1) ret.add(new InstructionHandle(new StackInstruction(Opcode.ADUP)));
				if (m==2) ret.add(new InstructionHandle(new StackInstruction(Opcode.ADUP2)));
				if (m>=3) throw new IllegalStateException(String.format("Cannot formulate adup instruction for m:%d, n:%d", m, n));
				break;
			case 1:
				// shorthand opcode for common case IDUP_X1
				if (m==1) ret.add(new InstructionHandle(new StackInstruction(Opcode.ADUP_X1)));
				if (m>=2) throw new IllegalStateException(String.format("Cannot formulate adup instruction for m:%d, n:%d", m, n));
				break;
			case 2:
				// shorthand opcode for common case IDUP_X2
				if (m==1) ret.add(new InstructionHandle(new StackInstruction(Opcode.ADUP_X2)));
				if (m>=2) throw new IllegalStateException(String.format("Cannot formulate adup instruction for m:%d, n:%d", m, n));
				break;
			default:
				throw new IllegalStateException(String.format("Cannot formulate adup instruction for m:%d, n:%d", m, n));
		}

		return ret;
	}

	private Collection<InstructionHandle> getIPopInstructions(int m)
	{
		ArrayList<InstructionHandle> ret = new ArrayList<InstructionHandle>(); 

		switch (m)
		{
			case 0:
				// no elements to pop, return the empty list
				break;
			case 1:
				ret.add(new InstructionHandle(new StackInstruction(Opcode.IPOP)));
				break;
			case 2:
				ret.add(new InstructionHandle(new StackInstruction(Opcode.IPOP2)));
				break;
			case 3:
				ret.add(new InstructionHandle(new StackInstruction(Opcode.IPOP)));
				ret.add(new InstructionHandle(new StackInstruction(Opcode.IPOP2)));
				break;
			case 4:
				ret.add(new InstructionHandle(new StackInstruction(Opcode.IPOP2)));
				ret.add(new InstructionHandle(new StackInstruction(Opcode.IPOP2)));
				break;
			default:
				throw new IllegalStateException(String.format("Cannot formulate integer pop opcode for m=%d", m));
		}
		
		return ret;
	}

	private Collection<InstructionHandle> getAPopInstructions(int m)
	{
		ArrayList<InstructionHandle> ret = new ArrayList<InstructionHandle>(); 

		if (m==1) ret.add(new InstructionHandle(new StackInstruction(Opcode.APOP)));
		if (m==2) ret.add(new InstructionHandle(new StackInstruction(Opcode.APOP2)));
		if (m>=3) throw new IllegalStateException(String.format("Cannot formulate reference pop opcode for m=%d", m));
		
		return ret;
	}	
	
	private Collection<InstructionHandle> getISwapInstructions(int m, int n)
	{
		ArrayList<InstructionHandle> ret = new ArrayList<InstructionHandle>(); 

		if ((m!=0)&&(n!=0)) ret.add(new InstructionHandle(new WideStackInstruction(Opcode.ISWAP_X, m, n)));
		
		return ret;
	}
	
	private Collection<InstructionHandle> getASwapInstructions(int m, int n)
	{
		ArrayList<InstructionHandle> ret = new ArrayList<InstructionHandle>(); 

		if ((m==1)&&(n==1)) ret.add(new InstructionHandle(new StackInstruction(Opcode.ASWAP)));
		
		return ret;
	}

	@Override
	public void transformInternal()
	{
		BaseType type1, type2, type3;
		
		InstructionList instructions = codeBlock.getInstructions();
		for (int i=0; i<instructions.size(); i++)
		{
			InstructionHandle handle = instructions.get(i);
			InterpreterState preState = handle.getPreState();
			
			switch (handle.getInstruction().getOpcode())
			{

				// generate the proper replacement for the IDUP instruction depending on the type inference information
				// ..., v1 -> ..., v1, v1
				case IDUP:

					type1 = preState.getStack().peek().getType();
					
					instructions.insertBefore(handle, getIDupInstructions(getNrIntegerSlots(type1), 0));
					instructions.insertBefore(handle, getADupInstructions(getNrReferenceSlots(type1), 0));
					instructions.remove(handle);
					
					break;
					
				// generate the proper replacement for the IDUP2 instruction depending on the type inference information
				// note that this instruction may generate a single integer dup instruction, a single reference dup instruction,
				// or one of both depending on the operands
				// ..., v2, v1 -> ..., v2, v1, v2, v1
				case IDUP2:
					type1 = preState.getStack().peek(0).getType();
					type2 = preState.getStack().peek(1).getType();
					
					instructions.insertBefore(handle, getIDupInstructions(getNrIntegerSlots(type1, type2), 0));
					instructions.insertBefore(handle, getADupInstructions(getNrReferenceSlots(type1, type2), 0));
					instructions.remove(handle);
					
					break;					

				// generate the proper replacement for the IPOP instruction depending on the type inference information
				case IPOP:
					
					type1 = preState.getStack().peek().getType();
					
					instructions.insertBefore(handle, getIPopInstructions(getNrIntegerSlots(type1)));
					instructions.insertBefore(handle, getAPopInstructions(getNrReferenceSlots(type1)));
					instructions.remove(handle);
					
					break;
					
				case IPOP2:
					try {
						type1 = preState.getStack().peek().getType();
					} catch (RuntimeException ex)
					{
						System.out.println(">>>> " + handle);
						
						for (InstructionHandle h : codeBlock.getInstructions().getInstructionHandles())
							System.out.println(h);
						throw ex;
					}
						
					type1 = preState.getStack().peek().getType();
					if (type1.isLongSized())
					{
						instructions.insertBefore(handle, getIPopInstructions(getNrIntegerSlots(type1)/2));
						instructions.remove(handle);
					} else
					{
						type2 = preState.getStack().peek(1).getType();
						instructions.insertBefore(handle, getIPopInstructions(getNrIntegerSlots(type1, type2)));
						instructions.insertBefore(handle, getAPopInstructions(getNrReferenceSlots(type1, type2)));
						instructions.remove(handle);
					}
					
					break;
					
				// generate the proper replacement for the IDUP_X1 instruction depending on the type inference information
				case IDUP_X1:
					type1 = preState.getStack().peek(0).getType();
					type2 = preState.getStack().peek(1).getType();
					
					instructions.insertBefore(handle, getIDupInstructions(getNrIntegerSlots(type1), getNrIntegerSlots(type2)));
					instructions.insertBefore(handle, getADupInstructions(getNrReferenceSlots(type1), getNrReferenceSlots(type2)));
					instructions.remove(handle);
					
					break;
					
				case IDUP_X2:
					
					type1 = preState.getStack().peek(0).getType();
					type2 = preState.getStack().peek(1).getType();
					type3 = preState.getStack().peek(2).getType();
					
					instructions.insertBefore(handle, getIDupInstructions(getNrIntegerSlots(type1), getNrIntegerSlots(type2, type3)));
					instructions.insertBefore(handle, getADupInstructions(getNrReferenceSlots(type1), getNrReferenceSlots(type2, type3)));
					instructions.remove(handle);
					
					break;
					
				case ISWAP_X:
					type1 = preState.getStack().peek(0).getType();
					type2 = preState.getStack().peek(1).getType();
					
					instructions.insertBefore(handle, getISwapInstructions(getNrIntegerSlots(type1), getNrIntegerSlots(type2)));
					instructions.insertBefore(handle, getASwapInstructions(getNrReferenceSlots(type1), getNrReferenceSlots(type2)));
					instructions.remove(handle);
					
					System.out.println("ERROR EMITTING SWAP!!!!\n");
					
					break;
					
				default:
					break;
			}
		}
	}

}
