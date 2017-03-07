/*
 * CodeBlockVisitor.java
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
 
package org.csiro.darjeeling.infuser.processingphase;

import org.csiro.darjeeling.infuser.logging.Logging;
import org.csiro.darjeeling.infuser.structure.DescendingVisitor;
import org.csiro.darjeeling.infuser.structure.Element;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalInfusion;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodImplementation;

public class CodeBlockVisitor extends DescendingVisitor
{
	
	private InternalInfusion infusion;
	
	public CodeBlockVisitor(InternalInfusion infusion)
	{
		this.infusion = infusion;
	}
	
	@Override
	public void visit(InternalMethodImplementation element)
	{
		Logging.instance.printlnVerbose(Logging.VerboseOutputType.BYTECODE_PROCESSING, String.format("processing codeblock for: %s.%s", element.getParentClass(), element.getMethodDefinition()));
		
		// Causes the CodeBlock element to be created
		// This is done after all the other elements in the tree have been resolved,
		// so that things like method definitions all have been assigned proper
		// global IDs.
		element.processCode(infusion);
	}

	@Override
	public void visit(Element element)
	{
	}

}
