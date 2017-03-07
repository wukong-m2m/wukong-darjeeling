/*
 * StackSizeVisitor.java
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
 
package org.csiro.darjeeling.infuser.outputphase;

import org.apache.bcel.classfile.Code;
import org.csiro.darjeeling.infuser.structure.DescendingVisitor;
import org.csiro.darjeeling.infuser.structure.Element;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalInfusion;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodImplementation;

public class StackSizeVisitor extends DescendingVisitor
{
	
	// private TreeMap<Float, Integer> distribution;
	private float sum = 0;
	private int methods, jvmTotal, dvmTotal, jvmTotalStack, jvmTotalLocals, dvmTotalStack, dvmTotalLocals;
	
	public StackSizeVisitor()
	{
		// distribution = new TreeMap<Float, Integer>();		
	}
	
	public void visit(InternalInfusion element)
	{
		visitChildren(element);
		
//		int total = 0;
//		for (Float reduction : distribution.keySet())
//			total += distribution.get(reduction);
//			
//		int sum = 0;
//		for (Float reduction : distribution.keySet())
//		{
//			sum += distribution.get(reduction);
//			System.out.printf("%f, %f\n", reduction, sum / (float)total );
//		}
		
		System.out.printf("%f, %f, %f, %f\n", 
				jvmTotalStack / (float)methods, 
				jvmTotalLocals / (float)methods,
				dvmTotalStack / (float)methods, 
				dvmTotalLocals / (float)methods
				);
		System.out.printf("avg reduction %f\n", ((jvmTotal-dvmTotal) * 100) / (float)jvmTotal );
		
	}
	
	public void visit(InternalMethodImplementation methodImplementation)
	{
		
		Code code = methodImplementation.getCode(); 
		if (code==null) return;
		
		int localVariables = 
			methodImplementation.getReferenceLocalVariableCount() - methodImplementation.getReferenceArgumentCount() - (methodImplementation.isStatic()?0:1) + 
			methodImplementation.getIntegerLocalVariableCount() - methodImplementation.getIntegerArgumentCount();

		int dvmStack = methodImplementation.getMaxStack() * 2;
		int dvmLocals = localVariables * 2;

		int jvmStack = code.getMaxStack() * 4;
		int jvmLocals = (code.getMaxLocals() - methodImplementation.getMethodDefinition().getParameterCount() - (methodImplementation.isStatic()?0:1) ) * 4;
		
		int dvm = dvmStack + dvmLocals;
		int jvm = jvmStack + jvmLocals;
		
		float reduction;
		
		if ((jvm-dvm)==0||jvm==0)
			reduction = 0;
		else
			reduction = ((jvm-dvm) * 100) / (float)jvm;
		
		sum += reduction;
		methods ++;
		
		jvmTotalStack += jvmStack;
		jvmTotalLocals += jvmLocals;
		jvmTotal += jvm;
		dvmTotalStack += dvmStack;
		dvmTotalLocals += dvmLocals;
		dvmTotal += dvm;
		
//		if (distribution.containsKey(reduction))
//			distribution.put(reduction, distribution.get(reduction) + 1);
//		else
//			distribution.put(reduction, 1);
//		
//		System.out.println(String.format("%d, %d, %d, %d, %d",
//				dvmStack,
//				jvmStack,
//				dvmLocals,
//				jvmLocals,
//				((jvm-dvm) * 100) / jvm 
//				));
		
	}

	@Override
	public void visit(Element element)
	{
	}

}
