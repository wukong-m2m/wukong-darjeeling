/*
 * TableSwitchInstruction.java
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

import org.csiro.darjeeling.infuser.bytecode.Opcode;

public class TableSwitchInstruction extends SwitchInstruction
{

	private int low, high; 
	
	public TableSwitchInstruction(Opcode opcode, int branchAdress, int low, int high, int[] branchOffsets)
	{
		super(opcode, branchAdress);
		assert(branchOffsets.length==(high-low+1)) : "the number of brachoffsets is incorrect";
		this.low = low;
		this.high = high;
		this.switchAddresses = branchOffsets;
	}
	
	public int getLow()
	{
		return low;
	}
	
	public int getHigh()
	{
		return high;
	}
	
	public void dump(DataOutputStream out) throws IOException
	{
		super.dump(out);
		out.writeInt(low);
		out.writeInt(high);
		for (int i=0; i<switchAddresses.length; i++)
			out.writeShort(switchAddresses[i]);
	}

	public int getLength()
	{
		return 1 + 2 + 4 + 4 + switchAddresses.length*2;
	}

}
