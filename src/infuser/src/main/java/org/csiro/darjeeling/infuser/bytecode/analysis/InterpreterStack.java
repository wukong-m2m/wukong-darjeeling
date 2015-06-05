/*
 * InterpreterStack.java
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

import java.util.Stack;

import org.csiro.darjeeling.infuser.bytecode.InstructionHandle;

public class InterpreterStack implements Comparable<InterpreterStack>
{
	
	private Stack<GeneratedValueSet> stack;
	
	public InterpreterStack()
	{
		stack = new Stack<GeneratedValueSet>();
	}
	
	private InterpreterStack(Stack<GeneratedValueSet> stack)
	{
		this.stack = stack;
	}
	
	public void push(InstructionHandle handle, int index)
	{
		stack.push(new GeneratedValueSet());
		stack.peek().add(new GeneratedValue(handle, index));
	}
	
	public void push(InstructionHandle handle)
	{
		push(handle, 0);
	}
	
	public GeneratedValueSet pop()
	{
		return stack.pop();
	}

	public GeneratedValueSet peek()
	{
		return peek(0);
	}
	
	public GeneratedValueSet get(int index)
	{
		return stack.get(index);
	}
	
	public GeneratedValueSet peek(int depth)
	{
		return stack.get(size()-1-depth);
	}
	
	public static InterpreterStack merge(InterpreterStack a, InterpreterStack b)
	{
		// check for size equivalence 
		if (a.size()!=b.size()) throw new IllegalStateException("Stacks must be same size");
		
		// merge stacks
		InterpreterStack ret = new InterpreterStack();
		for (int i=0; i<a.size(); i++)
			ret.stack.push(GeneratedValueSet.merge(a.stack.get(i), b.stack.get(i)));
		
		return ret;		
	}
	
	public int size()
	{
		return stack.size();
	}
	
	public int indexOf(GeneratedValueSet element)
	{
		return stack.indexOf(element);
	}
	
	public String toString()
	{
		String ret = "";
		for (int i=0; i<stack.size(); i++)
			ret += (i>0?",":"") + stack.get(i);
		return ret;
	}
	
	public InterpreterStack clone()
	{
		Stack<GeneratedValueSet> cloneStack = new Stack<GeneratedValueSet>();
		cloneStack.addAll(stack);
		return new InterpreterStack(cloneStack);
	}

	public int compareTo(InterpreterStack other)
	{
		int ret;
		// check for size equivalence 
		if (stack.size()!=other.stack.size()) return 1;
		
		// check for each slot on the stack if each element in a is also in b and vice versa
		for (int i=0; i<stack.size(); i++)
			if ((ret=stack.get(i).compareTo(other.stack.get(i)))!=0) return ret;
			
		return 0;
	}

}
