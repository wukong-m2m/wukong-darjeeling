/*
 * MarkLoopInstruction.java
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

import java.util.ArrayList;
import java.io.DataOutputStream;
import java.io.IOException;

import org.csiro.darjeeling.infuser.bytecode.LocalVariable;
import org.csiro.darjeeling.infuser.bytecode.Instruction;
import org.csiro.darjeeling.infuser.bytecode.Opcode;

public class MarkLoopStartInstruction extends SimpleInstruction
{
	ArrayList<Integer> valuetags;

	public MarkLoopStartInstruction(Opcode opcode, ArrayList<Integer> valuetags)
	{
		super(opcode);
		this.valuetags = valuetags;
	}
	
	@Override
	public int getLength()
	{
		return 2 + 2 * this.valuetags.size();
	}

	@Override
	public void dump(DataOutputStream out) throws IOException
	{
		out.write(opcode.getOpcode());
		int size = this.valuetags.size();
		if (size > 16) {
			size = 16;
		} 
		out.write(size);
		for (int i=0; i<size; i++) {
			int valuetag = this.valuetags.get(i);
			// Big endian
			out.write((valuetag >> 8) & 0xFF);
			out.write((valuetag) & 0xFF);
		}
	}
}
