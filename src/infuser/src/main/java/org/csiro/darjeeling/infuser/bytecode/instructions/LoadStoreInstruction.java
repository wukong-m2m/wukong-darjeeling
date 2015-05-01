/*
 * LoadStoreInstruction.java
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

import static org.csiro.darjeeling.infuser.bytecode.Opcode.ALOAD;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.ALOAD_0;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.ALOAD_1;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.ALOAD_2;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.ALOAD_3;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.ASTORE;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.ASTORE_0;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.ASTORE_1;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.ASTORE_2;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.ASTORE_3;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.ILOAD;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.ILOAD_0;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.ILOAD_1;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.ILOAD_2;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.ILOAD_3;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.ISTORE;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.ISTORE_0;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.ISTORE_1;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.ISTORE_2;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.ISTORE_3;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.LLOAD;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.LLOAD_0;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.LLOAD_1;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.LLOAD_2;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.LLOAD_3;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.LSTORE;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.LSTORE_0;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.LSTORE_1;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.LSTORE_2;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.LSTORE_3;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.SLOAD_0;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.SLOAD_1;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.SLOAD_2;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.SLOAD_3;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.SSTORE_0;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.SSTORE_1;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.SSTORE_2;
import static org.csiro.darjeeling.infuser.bytecode.Opcode.SSTORE_3;

import java.io.DataOutputStream;
import java.io.IOException;

import org.csiro.darjeeling.infuser.bytecode.InstructionHandle;
import org.csiro.darjeeling.infuser.bytecode.LocalVariable;
import org.csiro.darjeeling.infuser.bytecode.Opcode;
import org.csiro.darjeeling.infuser.structure.BaseType;

public class LoadStoreInstruction extends LocalVariableInstruction
{

	public LoadStoreInstruction(Opcode opcode, LocalVariable localVariable)
	{
		super(opcode, localVariable);
		if ((opcode!=ALOAD)&&(opcode!=ASTORE)&&(opcode!=ILOAD)&&(opcode!=ISTORE)&&(opcode!=LLOAD)&&(opcode!=LSTORE))
			throw new IllegalStateException("LoadStoreInstruction should be constructed with ISTORE, ILOAD, LSTORE, LLOAD, ASTORE or ALOAD");
	}
	
	@Override
	public void dump(DataOutputStream out) throws IOException
	{
		int index = getIndex();
		
		// check if the index is not -1 (in which case there is a bug in the local variable
		// slot assignment code)
		if (index==-1)
			throw new IllegalStateException("Illegal slot index -1 in load/store instruction");
		
		// normal ILOAD, ALOAD, ISTORE, ASTORE instructions
		if (index>=4) {
			out.write(opcode.getOpcode());
			out.write(index);
		} else
		// shorthand versions (ILOAD_0 etc)
		{
			switch (opcode)
			{
				case ALOAD:
					if (index==0) out.write(ALOAD_0.getOpcode());
					if (index==1) out.write(ALOAD_1.getOpcode());
					if (index==2) out.write(ALOAD_2.getOpcode());
					if (index==3) out.write(ALOAD_3.getOpcode());
					break;
				case ASTORE:
					if (index==0) out.write(ASTORE_0.getOpcode());
					if (index==1) out.write(ASTORE_1.getOpcode());
					if (index==2) out.write(ASTORE_2.getOpcode());
					if (index==3) out.write(ASTORE_3.getOpcode());
					break;
				case ILOAD:
					if (index==0) out.write(ILOAD_0.getOpcode());
					if (index==1) out.write(ILOAD_1.getOpcode());
					if (index==2) out.write(ILOAD_2.getOpcode());
					if (index==3) out.write(ILOAD_3.getOpcode());
					break;
				case ISTORE:
					if (index==0) out.write(ISTORE_0.getOpcode());
					if (index==1) out.write(ISTORE_1.getOpcode());
					if (index==2) out.write(ISTORE_2.getOpcode());
					if (index==3) out.write(ISTORE_3.getOpcode());
					break;
				case LLOAD:
					if (index==0) out.write(LLOAD_0.getOpcode());
					if (index==1) out.write(LLOAD_1.getOpcode());
					if (index==2) out.write(LLOAD_2.getOpcode());
					if (index==3) out.write(LLOAD_3.getOpcode());
					break;
				case LSTORE:
					if (index==0) out.write(LSTORE_0.getOpcode());
					if (index==1) out.write(LSTORE_1.getOpcode());
					if (index==2) out.write(LSTORE_2.getOpcode());
					if (index==3) out.write(LSTORE_3.getOpcode());
					break;
				case SLOAD:
					if (index==0) out.write(SLOAD_0.getOpcode());
					if (index==1) out.write(SLOAD_1.getOpcode());
					if (index==2) out.write(SLOAD_2.getOpcode());
					if (index==3) out.write(SLOAD_3.getOpcode());
					break;
				case SSTORE:
					if (index==0) out.write(SSTORE_0.getOpcode());
					if (index==1) out.write(SSTORE_1.getOpcode());
					if (index==2) out.write(SSTORE_2.getOpcode());
					if (index==3) out.write(SSTORE_3.getOpcode());
					break;
				default:
					throw new IllegalStateException("LoadStoreInstruction should be created with ISTORE, ILOAD, SSTORE, SLOAD, ASTORE or ALOAD");
			}
						
		}
	}
	
	@Override
	public void setOptimisationHints(InstructionHandle handle)
	{
		// set optimization hint
		if (opcode.isStoreInstruction())
			if (opcode.isIntLoadStoreInstruction())
				handle.setOptimisationHint(0, getOperandTypes()[0]);
			else
				handle.setOptimisationHint(0, BaseType.Ref);
		
	}
	
	@Override
	public BaseType getLogicalOutputType(int index, InstructionHandle handle)
	{
		if (opcode.isRefLoadStoreInstruction()) 
			return BaseType.Ref;
		else
			return localVariable.getMaxIntType();
	}
	
	public int getIndex()
	{
		int index = -1;
		
		if (opcode.isRefLoadStoreInstruction())
			index = localVariable.getReferenceIndex();

		if (opcode.isIntLoadStoreInstruction())
			index = localVariable.getIntegerIndex();
			
		return index;
	}
	
	@Override
	public int getLength()
	{
		return getIndex()<4?1:2;
	}
	
	@Override
	public String toString()
	{
		int index = getIndex();
		if (index==-1)
			return String.format("%s<%d>", opcode.getName(), localVariable.getSlot() );
		else
			return String.format("%s(%d)", opcode.getName(), index );
	}

}
