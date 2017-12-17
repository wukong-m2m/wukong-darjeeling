package org.csiro.darjeeling.infuser.bytecode.transformations;

import org.csiro.darjeeling.infuser.bytecode.CodeBlock;
import org.csiro.darjeeling.infuser.bytecode.CodeBlockTransformation;
import org.csiro.darjeeling.infuser.bytecode.Instruction;
import org.csiro.darjeeling.infuser.bytecode.InstructionHandle;
import org.csiro.darjeeling.infuser.bytecode.InstructionList;
import org.csiro.darjeeling.infuser.bytecode.Opcode;
import org.csiro.darjeeling.infuser.bytecode.instructions.SimpleInstruction;

public class AddStartStopAOTMeasurement extends CodeBlockTransformation
{
	public AddStartStopAOTMeasurement(CodeBlock codeBlock)
	{
		super(codeBlock);
	}

	@Override
	protected void transformInternal()
	{
		System.err.println("AddStartStopAOTMeasurement " + codeBlock.getMethodImplementation().getMethodDefinition().getName());
		if (codeBlock.getMethodImplementation().getMethodDefinition().getName().equals("rtcbenchmark_measure_java_performance")) {
			InstructionList instructions = codeBlock.getInstructions();

			InstructionHandle handle;

			handle = instructions.get(0);
			InstructionHandle startMeasurementHandle = handle.copyToNewHandleWithSameStateAndLiveVariables(new SimpleInstruction (Opcode.START_AOT_MEASUREMENT));
			instructions.insertBefore(handle, startMeasurementHandle);

			for (int i=0; i<instructions.size(); i++)
			{


				handle = instructions.get(i);
				Instruction instruction = handle.getInstruction();

		System.err.println("   " + instruction.getOpcode());

				
				if (instruction.getOpcode().isReturn()) {
					InstructionHandle stopMeasurementHandle = handle.copyToNewHandleWithSameStateAndLiveVariables(new SimpleInstruction (Opcode.STOP_AOT_MEASUREMENT));
					instructions.insertBefore(handle, stopMeasurementHandle);
					i++;
				}
			}
		}
	}
}
