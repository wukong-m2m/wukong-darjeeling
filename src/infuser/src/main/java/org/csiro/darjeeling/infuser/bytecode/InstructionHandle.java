/*
 * InstructionHandle.java
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
 
package org.csiro.darjeeling.infuser.bytecode;

import java.util.ArrayList;
import java.util.Collections;

import org.csiro.darjeeling.infuser.bytecode.analysis.GeneratedValueSet;
import org.csiro.darjeeling.infuser.bytecode.analysis.InterpreterState;
import org.csiro.darjeeling.infuser.bytecode.analysis.LiveSet;
import org.csiro.darjeeling.infuser.structure.BaseType;

/**
 * An InstructionHandle object wraps a single DVM instruction. Lists of incoming handles (handles that may precede the instruction) and outgoing handles (instructions
 * that possibly follow this instruction) are kept. Instruction handles are used to allow insertion/deletion of instructions without having to recalculate instruction
 * addresses, and to store meta data such as inferred type information. 
 * 
 * @author Niels Brouwers
 */
public class InstructionHandle implements Comparable<InstructionHandle>
{
	
	private ArrayList<InstructionHandle> incomingHandles;
	private InstructionHandle nextHandle, branchHandle;
	private ArrayList<InstructionHandle> exceptionHandlers;
	private ArrayList<InstructionHandle> switchTargets;
	private int pc;
	private Instruction instruction;
	private InterpreterState preState, postState;
	private boolean keepOverflow, generatesOverflow;
	private LiveSet liveVariables;
	
	/**
	 * Constructs a new InstructionHandle from an instruction. 
	 * @param instruction
	 */
	public InstructionHandle(Instruction instruction)
	{
		this.instruction = instruction;
		this.incomingHandles = new ArrayList<InstructionHandle>();
		this.exceptionHandlers = new ArrayList<InstructionHandle>();
		this.switchTargets = new ArrayList<InstructionHandle>();
		this.liveVariables = new LiveSet();
	}
	
	public void setKeepOverflow(boolean keepOverflow)
	{
		this.keepOverflow = keepOverflow;
	}
	
	public boolean isKeepOverflow()
	{
		return keepOverflow;
	}
	
	public void setGeneratesOverflow(boolean generatesOverflow)
	{
		this.generatesOverflow = generatesOverflow;
	}
	
	public boolean generatesOverflow()
	{
		return generatesOverflow;
	}
	
	public void setPc(int pc)
	{
		this.pc = pc;
	}
	
	public int getPc()
	{
		return pc;
	}
	
	public LiveSet getLiveVariables()
	{
		return liveVariables;
	}
	
	public void clearStates()
	{
		this.preState = null;
		this.postState = null;		
	}
	
	public Instruction getInstruction()
	{
		return instruction;
	}
	
	public void setInstruction(Instruction instruction)
	{
		this.instruction = instruction;
	}
	
	public void addIncomingHandle(InstructionHandle handle)
	{
		incomingHandles.add(handle);
	}
	
	public void setNextHandle(InstructionHandle nextHandle)
	{
		this.nextHandle = nextHandle;
	}
	
	public void setBranchHandle(InstructionHandle branchHandle)
	{
		this.branchHandle = branchHandle;
	}
	
	public InstructionHandle getBranchHandle()
	{
		return branchHandle;
	}
	
	public void addSwitchTarget(InstructionHandle handle)
	{
		switchTargets.add(handle);
	}
	
	public ArrayList<InstructionHandle> getSwitchTargets()
	{
		return switchTargets;
	}
	
	public boolean mergePreState(InterpreterState newState)
	{
		boolean changed = false;
		if (this.preState==null)
		{
			this.preState = newState;
			changed = true;
		}
		else
		{
			InterpreterState newPreState = InterpreterState.merge(newState, this.preState);
			changed = this.preState.compareTo(newPreState)!=0;
			this.preState = newPreState;
		}
		return changed;
	}
	
	@SuppressWarnings("unchecked")
	public ArrayList<InstructionHandle> getIncomingHandles()
	{
		return (ArrayList<InstructionHandle>)incomingHandles.clone();
	}
	
	public void clearIncomingHandles()
	{
		incomingHandles.clear();
	}
	
	public ArrayList<InstructionHandle> getOutgoingHandles()
	{
		ArrayList<InstructionHandle> ret = new ArrayList<InstructionHandle>();
		
		if (nextHandle!=null) ret.add(nextHandle);
		if (branchHandle!=null) ret.add(branchHandle);
		ret.addAll(exceptionHandlers);
		ret.addAll(switchTargets);
		
		return ret;
	}
	
	public void replaceOutgoingHandle(InstructionHandle oldHandle, InstructionHandle newHandle)
	{
		if (nextHandle==oldHandle) nextHandle = newHandle;
		if (branchHandle==oldHandle) branchHandle = newHandle;
		Collections.replaceAll(exceptionHandlers, oldHandle, newHandle);
		Collections.replaceAll(switchTargets, oldHandle, newHandle);
	}
	
	public void setPreState(InterpreterState preState)
	{
		this.preState = preState;
	}
	
	public void setPostState(InterpreterState postState)
	{
		this.postState = postState;
	}
	
	public BaseType getOperandType(int nr)
	{
		return preState.getStack().peek(nr).getLogicalType();
	}
	
	public GeneratedValueSet getOperand(int nr)
	{
		return preState.getStack().peek(nr);
	}
	
	public void propagateOptimisationHints()
	{
		int size = Math.min(preState.getStack().size(), postState.getStack().size());
		for (int i=0; i<size; i++)
		{
			GeneratedValueSet pre = preState.getStack().get(i);
			GeneratedValueSet post = postState.getStack().get(i);
			if (pre.compareTo(post)==0 && post.getOptimizationHint()!=null && pre.getOptimizationHint()==null)
				pre.setOptimizationHint(post.getOptimizationHint());
			
		}
	}
	
	public void setOptimisationHint(int nr, BaseType type, boolean force)
	{
		preState.getStack().peek(nr).setOptimizationHint(type, force);
	}
	
	public void setOptimisationHint(int nr, BaseType type)
	{
		preState.getStack().peek(nr).setOptimizationHint(type, false);
	}
	
	public BaseType getOptimisationHint()
	{
		return postState.getStack().peek().getOptimizationHint();
	}
	
	public InterpreterState getPreState()
	{
		return preState;
	}
	
	public InterpreterState getPostState()
	{
		return postState;
	}
	
	public BaseType getOutputType(int index)
	{
		return instruction.getOuputType(index, this);
	}
	
	public BaseType getLogicalOutputType(int index)
	{
		return instruction.getLogicalOutputType(index, this);
	}
	
	public int compareTo(InstructionHandle other)
	{
		if (other.getPc()==this.pc) return 0;
		else return (other.getPc()<this.pc)?1:-1;
	}
	
	private String buildIncomingHandlesString()
	{
		String ret = ""; 
		for (int i=0; i<incomingHandles.size(); i++)
			ret += (i==0?"":",") + incomingHandles.get(i).getPc();
		return ret;
	}
	
	private String buildOutgoingHandlesString()
	{
		String ret = ""; 
		ArrayList<InstructionHandle> outgoingHandles = getOutgoingHandles();
		for (int i=0; i<outgoingHandles.size(); i++)
			ret += (i==0?"":",") + outgoingHandles.get(i).getPc();
		return ret;
	}
	
	private String buildLiveVariablesString()
	{
		String ret = "";
		
		boolean first = true;
		for (LocalVariable variable : liveVariables)
		{
			if (variable!=null)
			{
				ret += (first?"":",") + variable.getSlot();
				first = false;	
			}
		}
		return ret;
	}
	
	public String toString()
	{
		return String.format("%04d %-12s %-12s %-30s %-60s %-60s %1s%1s %s",
			pc,
			buildIncomingHandlesString(),
			buildOutgoingHandlesString(),
			instruction,
			preState,
			postState,
			generatesOverflow?"G":"",
			keepOverflow?"K":"",
			buildLiveVariablesString()
			// postState==null?"":postState.getLocalVariables().toString()
			);
	}

}
