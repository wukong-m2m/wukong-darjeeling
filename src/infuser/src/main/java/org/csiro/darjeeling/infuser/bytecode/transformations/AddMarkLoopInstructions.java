package org.csiro.darjeeling.infuser.bytecode.transformations;

import org.csiro.darjeeling.infuser.bytecode.CodeBlock;
import org.csiro.darjeeling.infuser.bytecode.CodeBlockTransformation;
import org.csiro.darjeeling.infuser.bytecode.Instruction;
import org.csiro.darjeeling.infuser.bytecode.InstructionHandle;
import org.csiro.darjeeling.infuser.bytecode.InstructionList;
import org.csiro.darjeeling.infuser.bytecode.Opcode;
import org.csiro.darjeeling.infuser.bytecode.instructions.MarkLoopInstruction;

public class AddMarkLoopInstructions extends CodeBlockTransformation
{
	public AddMarkLoopInstructions(CodeBlock codeBlock)
	{
		super(codeBlock);
	}

	private static void insertMARKLOOP(InstructionList instructions, InstructionHandle loopBeginBRTARGET, InstructionHandle loopEndBRANCH) {
		InstructionHandle markLoopHandle;
		markLoopHandle = new InstructionHandle(new MarkLoopInstruction(Opcode.MARKLOOP, true));
		instructions.insertBefore(loopBeginBRTARGET, markLoopHandle);	
		markLoopHandle = new InstructionHandle(new MarkLoopInstruction(Opcode.MARKLOOP, false));
		instructions.insertAfter(loopEndBRANCH, markLoopHandle);	
	}

	@Override
	protected void transformInternal()
	{
		// Add MARKLOOP instructions around each inner loop.
		// An inner loop if found when there is a branch that satisfies:
		//  1 the branch branches backwards (it's a loop)
		//  2 there are no other backward branches in the block between this branch and the branch target (it's the inner loop)
		//  3 there are no forward branches beyond the instruction following this branch (single point of exit)
		// insert a BRTARGET instruction in front of each branch target instruction
		// System.err.println("Sinterklaas.");

		InstructionList instructions = codeBlock.getInstructions();
		for (int i=0; i<instructions.size(); i++)
		{
			InstructionHandle loopEndHandle = instructions.get(i);
			Instruction instruction = loopEndHandle.getInstruction();
			
			if (instruction.getOpcode().isBranch()) {
				InstructionHandle loopBeginHandle = loopEndHandle.getBranchHandle();

				// System.err.println("Considering branch at: " + loopEndHandle.getPc() + ".");

				//  1 the branch branches backwards (it's a loop)
				if (loopEndHandle.getPc() < loopBeginHandle.getPc()) {
					// System.err.println("Reject: not a back branch.");
					// Discard this branch.
					continue;
				}

				//  2 there are no other backward branches in the block between this branch and the branch target (it's the inner loop)
				boolean reject = false;
				for (int j=0; j<instructions.size(); j++) {
					InstructionHandle handle = instructions.get(j);
					if (handle.getInstruction().getOpcode().isBranch()
							&& handle.getPc() < loopEndHandle.getPc()
							&& handle.getPc() > loopBeginHandle.getPc()
							&& handle.getPc() > handle.getBranchHandle().getPc()) {
						// System.err.println("Reject: inner back branch found at " + handle.getPc() + ".");
						// Discard this branch.
						reject = true;
						break;
					}
				}
				if (reject) {
					continue;
				}

				//  3 there are no forward branches beyond the instruction following this branch (single point of exit)
				reject = false;
				for (int j=0; j<instructions.size(); j++) {
					InstructionHandle handle = instructions.get(j);
					if (handle.getInstruction().getOpcode().isBranch()
							&& handle.getPc() > loopEndHandle.getPc()
							&& handle.getPc() < loopBeginHandle.getPc()
							&& handle.getBranchHandle().getPc() > (loopBeginHandle.getPc() + 1)) {
						// System.err.println("Reject: inner forward branch to beyond end of loop found at " + handle.getPc() + ".");
						// Discard this branch.
						continue;
					}
				}
				if (reject) {
					continue;
				}

				// System.err.println("Found inner loop from " + loopBeginHandle.getPc() + " to " + loopEndHandle.getPc() + ".");

				// Found an inner loop. Mark it.
				insertMARKLOOP(instructions, loopBeginHandle, loopEndHandle);
				i += 2; // Skip the MARKLOOP instructions we just added
			}
		}

		// System.err.println("Vliegtuig.");

	}
}
