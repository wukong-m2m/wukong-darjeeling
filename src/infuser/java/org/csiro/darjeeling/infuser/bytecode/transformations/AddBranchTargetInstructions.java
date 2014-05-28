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
				InstructionHandle originalBranchHandle = handle.getBranchHandle();
				if (originalBranchHandle.getInstruction().getOpcode() == Opcode.BRTARGET) {
					// This happens after a negative jump since all instructions shirt forward and we get to see the same instruction twice.
					// System.out.println("------ SKIPPED: branch already points to BRTARGET.");
					continue;
				} else if (instructions.previous(originalBranchHandle) != null // In case we branch to pc=0
					&& instructions.previous(originalBranchHandle).getInstruction().getOpcode() == Opcode.BRTARGET) {
					// There is already a branch target before this instruction.
					// This happens when multiple branches point to the same target instruction.
					// We only need to redirect the current branch to the BRTARGET instruction.
					handle.setBranchHandle(instructions.previous(originalBranchHandle));
					// System.out.println("------ SKIPPED: already has BRTARGET in front of it. just redirect.");
					// System.out.println("------ SKIPPED: " + handle);
				} else {
					// Insert a BRTARGET before the target handle of this branch
					InstructionHandle brtargetHandle = new InstructionHandle(new BranchTargetInstruction (Opcode.BRTARGET));
					instructions.insertBefore(originalBranchHandle, brtargetHandle);

					handle.setBranchHandle(brtargetHandle);
				}
				// System.out.println("------ AFTER");
				// for (InstructionHandle h : instructions.getInstructionHandles())
				// {
				// 	System.out.println(h);
				// }
				// System.out.println("------ AFTER");
			}
		}

		// Now number all branch targets
		int branchTargetCount = 0;
		for (int i=0; i<instructions.size(); i++)
		{
			InstructionHandle handle = instructions.get(i);
			
			Instruction instruction = handle.getInstruction();
			
			if (instruction.getOpcode() == Opcode.BRTARGET) {
				((BranchTargetInstruction)instruction).setBranchTargetIndex(branchTargetCount++);
			}
		}
	}
}
