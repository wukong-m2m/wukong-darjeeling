/*
 * WideIncreaseInstruction.java
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

import org.csiro.darjeeling.infuser.bytecode.LocalVariable;
import org.csiro.darjeeling.infuser.bytecode.Opcode;

public class WideIncreaseInstruction extends LocalVariableInstruction
{
	
	protected int value;

	public WideIncreaseInstruction(Opcode opcode, LocalVariable localVariable, int value)
	{
		super(opcode, localVariable);
		if ((value>Short.MAX_VALUE)||(value<Short.MIN_VALUE))
			throw new IllegalArgumentException("value is out of range");
		this.value = value;
	}
	
	@Override
	public int getLength()
	{
		return 4;
	}
	
	@Override
	public void dump(DataOutputStream out) throws IOException
	{
		out.write(opcode.getOpcode());
		out.write(localVariable.getIntegerIndex());
		out.writeShort(value);
	}

}
