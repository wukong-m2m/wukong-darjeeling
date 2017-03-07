/*
 * CHeaderVisitor.java
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

import java.io.PrintWriter;

import org.csiro.darjeeling.infuser.bytecode.CodeBlock;
import org.csiro.darjeeling.infuser.bytecode.InstructionHandle;
import org.csiro.darjeeling.infuser.structure.DescendingVisitor;
import org.csiro.darjeeling.infuser.structure.Element;
import org.csiro.darjeeling.infuser.structure.elements.AbstractHeader;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalClassDefinition;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalInfusion;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodImplementation;

/**
 * 
 * Outputs a C header file with #define statements linking human readable names such as 'BASE_CDEF_java_lang_Object' with generated entity IDs.
 * 
 * @author Niels Brouwers
 *
 */
public class DebugVisitor extends DescendingVisitor
{
	
	private PrintWriter writer;
	private String infusionName;
	
	public DebugVisitor(PrintWriter writer)
	{
		this.writer = writer;
	}
	
	@Override
	public void visit(InternalInfusion element)
	{
		AbstractHeader header = element.getHeader();
		
		writer.println(String.format("infusion %s", header.getInfusionName()));
		writer.println("{");
		writer.println("");
		
		super.visit(element);
		
		writer.println("");
		writer.println("}");
	}
	
	public void visit(InternalClassDefinition element)
	{
		String interfaces = element.getInterfaces().size()==0 ? "" : "implements";
		for (String interfaceName : element.getInterfaceNames())
			interfaces += " " + interfaceName;
		
		writer.println(String.format("\tclass %s extends %s %s", element.getName(), element.getSuperClassName(), interfaces));
		writer.println("\t{");
		writer.println("");
		
		super.visit(element);
		
		writer.println("");
		writer.println("\t}");
		
	}

	public void visit(InternalMethodImplementation element)
	{
		writer.println(String.format("\tmethod %s", element.getMethodDefinition().toString()));
		writer.println("\t{");
		writer.println("");
		
		CodeBlock code = element.getCodeBlock();
		if (code!=null)
			for (InstructionHandle handle : code.getInstructions().getInstructionHandles())
			{
				writer.println("\t\t" + handle.toString());
			}
		
		writer.println("");
		writer.println("\t}");
	}
	
	@Override
	public void visit(Element element)
	{
	}

}
