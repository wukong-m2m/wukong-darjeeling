/*
 * StackInstruction.java
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
import org.csiro.darjeeling.infuser.bytecode.analysis.GeneratedValueSet;
import org.csiro.darjeeling.infuser.structure.BaseType;

public class StackInstruction extends SimpleInstruction
{

	public StackInstruction(Opcode opcode)
	{
		super(opcode);
	}
	
	private GeneratedValueSet getOuputElement(int index, InstructionHandle handle)
	{
		GeneratedValueSet element = null;
		switch (opcode)
		{
			case IPOP:
				throw new IllegalStateException("POP does not output values");
				
			case IPOP2:
				throw new IllegalStateException("POP does not output values");
				
			case IDUP:
				if (index<2) element = handle.getPreState().getStack().peek();
				else throw new IllegalStateException("DUP outputs only two values");
				break;
				
			case IDUP2:
				if (index<4) element = handle.getPreState().getStack().peek(index%2); 
				else throw new IllegalStateException("DUP2 outputs only four values");
				break;
				
			case IDUP_X1:
				if (index<3) element = handle.getPreState().getStack().peek(index%2); 
				else throw new IllegalStateException("DUP_X1 outputs only three values");
				break;
				
			case IDUP_X2:
				if (index<4) element = handle.getPreState().getStack().peek(index%3); 
				else throw new IllegalStateException("DUP_X2 outputs only four values");
				break;
				
			case ISWAP_X:
				if (index<2) element = handle.getPreState().getStack().peek((index+1)%2); 
				else throw new IllegalStateException("SWAP outputs only two values");
				break;
				
			default:
				throw new IllegalStateException("Opcode is not a valid stack type");
		}
		return element;
	}
	
	@Override
	public int getNrOutputValues()
	{
		switch (opcode)
		{
			case IPOP: return 0;
			case IPOP2: return 0;				
			case IDUP: return 2;
			case IDUP2: return 4;
			case IDUP_X1: return 3;
			case IDUP_X2: return 4;
			case ISWAP_X: return 2;
				
			default:
				throw new IllegalStateException("Unsupported stack operand");
		}
	}
	
	@Override
	public BaseType getOuputType(int index, InstructionHandle handle)
	{
		return getOuputElement(index, handle).getType();
	}

	@Override
	public BaseType getLogicalOutputType(int index, InstructionHandle handle)
	{
		return getOuputElement(index, handle).getLogicalType();
	}
	
	private BaseType max(BaseType a, BaseType b)
	{
		if ((a==null)||(b==null)||(a==BaseType.Ref)||(b==BaseType.Ref)) return null;
		return BaseType.max(a,b);
	}
	
	@Override
	public void setOptimisationHints(InstructionHandle handle)
	{
		// set optimization hints
		BaseType hinta,hintb;
		switch (opcode)
		{
			case IPOP:
				// since we're popping the operand, we don't care about its type
				handle.setOptimisationHint(0, BaseType.DontCare);
				break;
				
			case IPOP2:
				// since we're popping the operands, we don't care about their types type
				// TODO cleanup
				if (handle.getPreState().getStack().size()==1)
				{
					handle.setOptimisationHint(0, BaseType.DontCare);
				} else
				{
					handle.setOptimisationHint(0, BaseType.DontCare);
					handle.setOptimisationHint(1, BaseType.DontCare);
				}
				break;
				
			case IDUP:
				hinta = handle.getPostState().getStack().peek(1).getOptimizationHint();
				hintb = handle.getPostState().getStack().peek(0).getOptimizationHint();
				handle.setOptimisationHint(0, max(hinta, hintb));
				break;
				
			case IDUP2:
				hinta = handle.getPostState().getStack().peek(2).getOptimizationHint();
				hintb = handle.getPostState().getStack().peek(0).getOptimizationHint();
				handle.setOptimisationHint(0, max(hinta, hintb));

				hinta = handle.getPostState().getStack().peek(3).getOptimizationHint();
				hintb = handle.getPostState().getStack().peek(1).getOptimizationHint();
				handle.setOptimisationHint(1, max(hinta, hintb));
				break;
				
			case IDUP_X1:
				// b,a -> a,b,a
				hinta = handle.getPostState().getStack().peek(2).getOptimizationHint();
				hintb = handle.getPostState().getStack().peek(0).getOptimizationHint();
				handle.setOptimisationHint(0, max(hinta, hintb));
				hinta = handle.getPostState().getStack().peek(1).getOptimizationHint();
				handle.setOptimisationHint(1, hinta);
				break;
				
			case IDUP_X2:
				// c,b,a -> a,c,b,a
				hinta = handle.getPostState().getStack().peek(3).getOptimizationHint();
				hintb = handle.getPostState().getStack().peek(0).getOptimizationHint();
				handle.setOptimisationHint(0, max(hinta, hintb));

				hinta = handle.getPostState().getStack().peek(1).getOptimizationHint();
				handle.setOptimisationHint(1, hinta);
				
				hinta = handle.getPostState().getStack().peek(2).getOptimizationHint();
				handle.setOptimisationHint(2, hinta);
				break;
				
			case ISWAP_X:
				hinta = handle.getPostState().getStack().peek(1).getOptimizationHint();
				hintb = handle.getPostState().getStack().peek(0).getOptimizationHint();
				handle.setOptimisationHint(0, hinta);
				handle.setOptimisationHint(1, hintb);
				break;
				
			default:
				throw new IllegalStateException("Opcode is not a valid stack type");
		}
	}
	

}
