/*
 * XHTMLDebugVisitor.java
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

import org.csiro.darjeeling.infuser.bytecode.CodeBlock;
import org.csiro.darjeeling.infuser.bytecode.InstructionHandle;
import org.csiro.darjeeling.infuser.structure.DescendingVisitor;
import org.csiro.darjeeling.infuser.structure.Element;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodImplementation;
import org.w3c.dom.Document;

public class XHTMLDebugVisitor extends DescendingVisitor
{
	
	private Document doc;
	private org.w3c.dom.Element currentElement;

	public XHTMLDebugVisitor(Document doc)
	{
		this.doc = doc;
		
		// create root node
		org.w3c.dom.Element xhtml = doc.createElement("html");
		doc.appendChild(xhtml);

		org.w3c.dom.Element body = doc.createElement("body");
		xhtml.appendChild(body);
		
		currentElement = body;
	}

//	public void visit(InternalClassDefinition element)
//	{
//		org.w3c.dom.Element h1 = doc.createElement("H1");
//		
//		String classString = element.getClassName();
//		String superClassName = element.getSuperClassName();
//		if (!superClassName.equals("java.lang.Object")) classString += " extends " + superClassName;
//		h1.setTextContent(classString);
//		
//		currentElement.appendChild(h1);
//		
//		super.visit(element);
//	}
	
	public void visit(InternalMethodImplementation element)
	{
		org.w3c.dom.Element h1, table, tr, td;
		h1 = doc.createElement("H2");
		table = doc.createElement("table");
		
		String methodString = element.getParentClass().getName() + "::" + element.getMethodDefinition().getName() + " impl" + element.getGlobalId()+ " def" + element.getMethodDefinition().getGlobalId();
		h1.setTextContent(methodString);
		
		// generate code
		CodeBlock codeBlock = element.getCodeBlock();
		for (InstructionHandle handle : codeBlock.getInstructions().getInstructionHandles())
		{
			tr = doc.createElement("tr");
			
			// PC TD
			td = doc.createElement("td");
			td.setTextContent("" + handle.getPc());
			tr.appendChild(td);
			
			// Instruction TD
			td = doc.createElement("td");
			td.setTextContent("" + handle.getInstruction().toString());
			tr.appendChild(td);
			
			table.appendChild(tr);
		}
		
		currentElement.appendChild(h1);
		currentElement.appendChild(table);

		super.visit(element);
	}
	
	public void visit(Element element)
	{
	}
	
}
