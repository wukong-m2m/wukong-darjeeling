package org.csiro.darjeeling.infuser.bytecode.transformations;

import org.csiro.darjeeling.infuser.bytecode.CodeBlock;
import org.csiro.darjeeling.infuser.bytecode.CodeBlockTransformation;
import org.csiro.darjeeling.infuser.bytecode.Instruction;
import org.csiro.darjeeling.infuser.bytecode.InstructionHandle;
import org.csiro.darjeeling.infuser.bytecode.InstructionList;
import org.csiro.darjeeling.infuser.bytecode.Opcode;
import org.csiro.darjeeling.infuser.bytecode.instructions.BranchTargetInstruction;

public class AddBranchTargetInstructions extends CodeBlockTransformation
{
	public AddBranchTargetInstructions(CodeBlock codeBlock)
	{
		super(codeBlock);
	}

	private static InstructionHandle insertBRTARGETbefore(InstructionList instructions,
											 InstructionHandle targetInstructionHandle) {
		if (targetInstructionHandle.getInstruction().getOpcode() == Opcode.BRTARGET) {
			// This happens after a negative jump since all instructions shift forward and we get to see the same instruction twice.
			// System.out.println("------ SKIPPED: branch already points to BRTARGET.");

			return targetInstructionHandle; // this will be the new branch target

		} else if (instructions.previous(targetInstructionHandle) != null // In case we branch to pc=0
			&& instructions.previous(targetInstructionHandle).getInstruction().getOpcode() == Opcode.BRTARGET) {
			// There is already a branch target right before this instruction.
			// This happens when multiple branches point to the same target instruction.
			// We only need to redirect the current branch to the BRTARGET instruction.

			return instructions.previous(targetInstructionHandle); // this will be the new branch target
			
			// System.out.println("------ SKIPPED: already has BRTARGET in front of it. just redirect.");
			// System.out.println("------ SKIPPED: " + handle);
		} else {
			// Insert a BRTARGET before the target handle of this branch
			InstructionHandle brtargetHandle = new InstructionHandle(new BranchTargetInstruction (Opcode.BRTARGET));
			instructions.insertBefore(targetInstructionHandle, brtargetHandle);

			return brtargetHandle; // this will be the new branch target
		}		
	}

	@Override
	protected void transformInternal()
	{
		// insert a BRTARGET instruction in front of each branch target instruction
		InstructionList instructions = codeBlock.getInstructions();
		for (int i=0; i<instructions.size(); i++)
		{
			InstructionHandle handle = instructions.get(i);
			Instruction instruction = handle.getInstruction();
			
			if (instruction.getOpcode().isBranch()) {
				// System.out.println("------ BEFORE");
				// for (InstructionHandle h : instructions.getInstructionHandles())
				// {
				// 	System.out.println(h);
				// }
				// System.out.println("------ BEFORE");

				// System.out.println("------ BRANCH " + handle);
				// System.out.println("------ NEXTHANDLE " + handle.getNextHandle());
				// System.out.println("------ BRANCHHANDLE " + handle.getBranchHandle());
				InstructionHandle targetInstructionHandle = handle.getBranchHandle();
				InstructionHandle newTargetInstructionHandle = insertBRTARGETbefore(instructions, targetInstructionHandle);
				handle.setBranchHandle(newTargetInstructionHandle);
				// System.out.println("------ AFTER");
				// for (InstructionHandle h : instructions.getInstructionHandles())
				// {
				// 	System.out.println(h);
				// }
				// System.out.println("------ AFTER");
			}

			if (instruction.getOpcode().isSwitch()) {
				for (int j=0; j<handle.getSwitchTargets().size(); j++) {
					InstructionHandle targetInstructionHandle = handle.getSwitchTargets().get(j);
					InstructionHandle newTargetInstructionHandle = insertBRTARGETbefore(instructions, targetInstructionHandle);
					handle.getSwitchTargets().set(j, newTargetInstructionHandle);
				}
			}
		}

		// Numbering the branch targets will be done in InstructionList, since AddMarkLoopInstructions may add some more.
	}
}
