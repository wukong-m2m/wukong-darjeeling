package org.csiro.darjeeling.infuser.bytecode;

import org.csiro.darjeeling.infuser.structure.BaseType;
import org.csiro.darjeeling.infuser.bytecode.instructions.LightweightMethodParameterInstruction;

// Dummy handle to put a lightweight method's parameters on the stack.
public class LightweightMethodParameterHandle extends InstructionHandle
{
	public LightweightMethodParameterHandle(Opcode opcode)
	{
		super(new LightweightMethodParameterInstruction(opcode));
	}
}
