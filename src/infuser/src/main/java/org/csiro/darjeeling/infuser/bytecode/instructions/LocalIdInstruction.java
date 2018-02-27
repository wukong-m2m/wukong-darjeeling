/*
 * LocalIdInstruction.java
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
import org.csiro.darjeeling.infuser.structure.LocalId;
import org.csiro.darjeeling.infuser.structure.elements.AbstractField;

public class LocalIdInstruction extends Instruction
{
	
	private LocalId localId = null;
	private AbstractField field = null;

	public LocalIdInstruction(Opcode opcode, LocalId localId)
	{
		super(opcode);
		this.localId = localId;
	}
	public LocalIdInstruction(Opcode opcode, LocalId localId, AbstractField field)
	{
		super(opcode);
		this.localId = localId;
		this.field = field;
	}


	@Override
	public void dump(DataOutputStream out) throws IOException
	{
		out.write(opcode.getOpcode());
		out.writeByte(localId.getInfusionId());
		out.writeByte(localId.getLocalId());
	}

	public LocalId getLocalId() {
		return this.localId;
	}

	public AbstractField getField() {
		return this.field;
	}

	@Override
	public int getLength()
	{
		return 3;
	}
	
	@Override
	public String toString()
	{
		return opcode.getName() + "(" + localId.getInfusionId() + "," + localId.getLocalId() + ")";
	}
}
