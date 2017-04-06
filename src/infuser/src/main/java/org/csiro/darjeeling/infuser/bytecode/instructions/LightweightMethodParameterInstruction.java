package org.csiro.darjeeling.infuser.bytecode.instructions;

import java.io.DataOutputStream;
import java.io.IOException;

import org.csiro.darjeeling.infuser.bytecode.Instruction;
import org.csiro.darjeeling.infuser.bytecode.Opcode;

// Dummy handle to put a lightweight method's parameters on the stack.
public class LightweightMethodParameterInstruction extends Instruction
{

	public LightweightMethodParameterInstruction(Opcode opcode)
	{
		super(opcode);
	}

	@Override
	public void dump(DataOutputStream out) throws IOException
	{
		// We shouldn't generate any code.
		// This instruction is just to trick the infuser in putting the parameters on the stack for lightweight methods.
	}

	@Override
	public int getLength()
	{
		return 0;
	}
}
