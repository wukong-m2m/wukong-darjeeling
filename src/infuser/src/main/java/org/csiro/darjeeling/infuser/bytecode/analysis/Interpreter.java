/*
 * Interpreter.java
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
 
package org.csiro.darjeeling.infuser.bytecode.analysis;

import java.util.ArrayList;
import java.util.TreeSet;

import org.csiro.darjeeling.infuser.bytecode.CodeBlock;
import org.csiro.darjeeling.infuser.bytecode.DummyHandle;
import org.csiro.darjeeling.infuser.bytecode.ExceptionHandler;
import org.csiro.darjeeling.infuser.bytecode.Instruction;
import org.csiro.darjeeling.infuser.bytecode.InstructionHandle;
import org.csiro.darjeeling.infuser.bytecode.InstructionList;
import org.csiro.darjeeling.infuser.bytecode.Opcode;
import org.csiro.darjeeling.infuser.bytecode.instructions.LocalVariableInstruction;

public class Interpreter
{
	
	private ArrayList<InstructionHandle> pendingHandles;
	
	private CodeBlock codeBlock;
	
	public Interpreter(CodeBlock codeBlock)
	{
		this.codeBlock = codeBlock;
		this.pendingHandles = new ArrayList<InstructionHandle>(); 
	}

	private InstructionHandle pickNextState()
	{
		if (pendingHandles.size()>0) return pendingHandles.get(0);
		
		return null;
	}
	
	public void inferTypes()
	{
		pendingHandles.clear();

		// clear the interpreter state on all the handles (clean slate)
		for (InstructionHandle localHandle : codeBlock.getInstructions().getInstructionHandles())
			localHandle.clearStates();

		// make sure we don't break if the number of instructions in the code block is 0
		if (codeBlock.getInstructions().size()==0) return;
		
		// bootstrap the first instruction
		InstructionHandle firstInstruction = codeBlock.getInstructions().get(0);
		firstInstruction.setPreState(new InterpreterState());
		pendingHandles.add(firstInstruction);

		// Bootstrap all the exception handlers. Each instruction handler should be
		// entered with a single REF value on the stack
		for (ExceptionHandler handler : codeBlock.getExceptionHandlers())
		{
			InstructionHandle handlerInstruction = handler.getHandler();
			InterpreterState exceptionState = new InterpreterState();
			
			// insert dummy throw handle 
			exceptionState.getStack().push(new DummyHandle(handler.getStart()));
			
			handlerInstruction.setPreState(exceptionState);
			pendingHandles.add(handlerInstruction);			
		}
		
		// interpret
		InstructionHandle handle = pickNextState();
		while ((handle=pickNextState())!=null)
		{
			// intepret this instruction
			InterpreterState postState = handle.getPreState().transition(handle);
			handle.setPostState(postState);

			// merge the pre-states of all the outgoing handles with this new post-state
			for (InstructionHandle outgoingHandle : handle.getOutgoingHandles())
			{
				try {
					boolean changed = outgoingHandle.mergePreState(postState);
					if (changed) pendingHandles.add(outgoingHandle);
				} catch (IllegalArgumentException ex)
				{
					throw new IllegalStateException(String.format("Unable to merge stacks at pc=%d: %s", handle.getPc(), ex.getMessage() ));
				}
			}
			
			pendingHandles.remove(handle);
		}
		
	}
	
	/**
	 * 
	 */
	public void inferLiveRanges()
	{
		
		InstructionList instructions = codeBlock.getInstructions();
		
		// clear the live range information on all the handles
		for (InstructionHandle localHandle : instructions.getInstructionHandles())
			localHandle.getLiveVariables().clear();

		// make sure we don't break if the number of instructions in the code block is 0
		if (codeBlock.getInstructions().size()==0) return;
		
		pendingHandles.clear();
		
		// we first find the last instructions in the method,
		// then we add the last instruction in each try block,
		// the reason to consider the try blocks separately is if
		// a try block is followed by no instruction in a simple while loop,
		// the bottom-to-top chain of incoming handle is lost in the catch instructions
		// bootstrap the last instruction
		pendingHandles.add(instructions.get(instructions.size()-1));
		
		// bootstrap the last instruction in all try blocks
		for (ExceptionHandler exceptionHandler : codeBlock.getExceptionHandlers())
		{
			InstructionHandle lastHandlerInstruction = codeBlock.getInstructions().previous(exceptionHandler.getEnd());
			if (lastHandlerInstruction!=null)
				pendingHandles.add(lastHandlerInstruction);
		}

		for (InstructionHandle currentHandle : pendingHandles)
			currentHandle.getLiveVariables().add(null);

		TreeSet<InstructionHandle> visitedHandles = new TreeSet<InstructionHandle>();

		// interpret
		InstructionHandle handle = pickNextState();
		while ((handle=pickNextState())!=null)
		{
			visitedHandles.add(handle);
			Instruction instruction = handle.getInstruction();
			
			// load or iinc instruction (mark this handle as live)
			if (instruction.getOpcode().isLoadInstruction() || 
					instruction.getOpcode()==Opcode.IINC || 
					instruction.getOpcode()==Opcode.IINC_W ||
					instruction.getOpcode()==Opcode.SINC || 
					instruction.getOpcode()==Opcode.SINC_W
					)
				handle.getLiveVariables().add(((LocalVariableInstruction)instruction).getLocalVariable());
			
			// store instruction (unmark live)
			if (instruction.getOpcode().isStoreInstruction())
				handle.getLiveVariables().remove(((LocalVariableInstruction)instruction).getLocalVariable());
			
			// merge the states of all the incoming handles with the new state
			for (InstructionHandle incomingHandle : handle.getIncomingHandles())
			{
				try {
					boolean changed = incomingHandle.getLiveVariables().merge(handle.getLiveVariables());
					if (!visitedHandles.contains(incomingHandle) || changed) pendingHandles.add(incomingHandle);
				} catch (IllegalArgumentException ex)
				{
					throw new IllegalStateException(String.format("Unable to merge stacks at pc=%d: %s", handle.getPc(), ex.getMessage() ));
				}
			}				
				
			// hack hack hack
			if (instruction.getOpcode().isStoreInstruction())
				handle.getLiveVariables().add(((LocalVariableInstruction)instruction).getLocalVariable());
			
			
			pendingHandles.remove(handle);
		}
		
	}
	
}
