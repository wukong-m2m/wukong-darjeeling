/*
 * StaticInvokeInstruction.java
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

import org.csiro.darjeeling.infuser.bytecode.Opcode;
import org.csiro.darjeeling.infuser.structure.LocalId;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodImplementation;
import org.csiro.darjeeling.infuser.structure.lightweightmethods.LightweightMethod;

public class StaticInvokeInstruction extends AbstractInvokeInstruction
{
	AbstractMethodImplementation methodImplementation;
	LightweightMethod lightweightMethod;

	public StaticInvokeInstruction(Opcode opcode, LocalId localId, AbstractMethodImplementation methodImplementation)
	{
		super(opcode, localId, methodImplementation.getMethodDefinition());
		this.methodImplementation = methodImplementation;
	}

	public AbstractMethodImplementation getMethodImplementation() {
		return this.methodImplementation;
	}

	public void setLightweightMethod(LightweightMethod lightweightMethod) {
		this.lightweightMethod = lightweightMethod;
	}

	public LightweightMethod getLightweightMethod() {
		return this.lightweightMethod;
	}
}
