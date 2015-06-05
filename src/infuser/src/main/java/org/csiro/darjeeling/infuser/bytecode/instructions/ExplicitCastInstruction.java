/*
 * ExplicitCastInstruction.java
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
import org.csiro.darjeeling.infuser.structure.BaseType;
import org.csiro.darjeeling.infuser.structure.TypeClass;

public class ExplicitCastInstruction extends SimpleInstruction
{

	public ExplicitCastInstruction(Opcode opcode)
	{
		super(opcode);
	}
	
	@Override
	public void setOptimisationHints(InstructionHandle handle)
	{
		BaseType hint = handle.getOptimisationHint();

		if (hint.getTypeClass()==TypeClass.Short)
			switch (opcode)
			{
				case I2B:
					opcode = Opcode.S2B;
					handle.setOptimisationHint(0, BaseType.Short);
					break; 
				case I2C:
					opcode = Opcode.S2C;
					handle.setOptimisationHint(0, BaseType.Short);
					break; 
				case I2S:
					opcode = Opcode.NOP;
					handle.setOptimisationHint(0, BaseType.Short);
					break;
			}
		else
			super.setOptimisationHints(handle);
			
	}

}
