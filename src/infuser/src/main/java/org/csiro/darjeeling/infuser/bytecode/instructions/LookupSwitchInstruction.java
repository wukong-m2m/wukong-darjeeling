/*
 * LookupSwitchInstruction.java
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

public class LookupSwitchInstruction extends SwitchInstruction
{

	protected int values[];
	
	public LookupSwitchInstruction(Opcode opcode, int branchAdress, int values[], int targets[])
	{
		super(opcode, branchAdress);
		assert(targets.length==values.length) : "value and target adress arrays should be of the same size";
		this.values = values;
		this.switchAddresses = targets;
	}
	
	public void dump(DataOutputStream out) throws IOException
	{
		super.dump(out);
		out.writeShort(values.length);
		for (int i=0; i<values.length; i++)
		{
			out.writeInt(values[i]);
			out.writeShort(switchAddresses[i]);
		}
	}
	
	public int getLength()
	{
		return 1 + 2 + 2 + values.length * 4 + switchAddresses.length * 2;
	}

}
