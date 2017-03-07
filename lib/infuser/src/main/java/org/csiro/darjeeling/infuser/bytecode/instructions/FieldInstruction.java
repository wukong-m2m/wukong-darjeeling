/*
 * FieldInstruction.java
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

import java.io.DataOutputStream;
import java.io.IOException;

import org.csiro.darjeeling.infuser.bytecode.Instruction;
import org.csiro.darjeeling.infuser.bytecode.Opcode;

public class FieldInstruction extends Instruction
{

	private int offset;
	
	public FieldInstruction(Opcode opcode, int offset)
	{
		super(opcode);
		this.offset = offset;
	}

	@Override
	public void dump(DataOutputStream out) throws IOException
	{
		out.write(opcode.getOpcode());
		out.writeShort(offset);
	}

	@Override
	public int getLength()
	{
		return 3;
	}

}
