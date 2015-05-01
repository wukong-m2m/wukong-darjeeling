/*
 * HeaderResolutionVisitor.java
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

import org.csiro.darjeeling.infuser.structure.DescendingVisitor;
import org.csiro.darjeeling.infuser.structure.Element;
import org.csiro.darjeeling.infuser.structure.elements.AbstractField;
import org.csiro.darjeeling.infuser.structure.elements.AbstractInfusion;
import org.csiro.darjeeling.infuser.structure.elements.external.ExternalClassDefinition;
import org.csiro.darjeeling.infuser.structure.elements.external.ExternalField;
import org.csiro.darjeeling.infuser.structure.elements.external.ExternalMethod;
import org.csiro.darjeeling.infuser.structure.elements.external.ExternalMethodImplementation;

public class HeaderResolutionVisitor extends DescendingVisitor
{

	private AbstractInfusion rootInfusion;
	
	public HeaderResolutionVisitor(AbstractInfusion rootInfusion)
	{
		this.rootInfusion = rootInfusion;
	}
	
	@Override
	public void visit(ExternalMethodImplementation element)
	{
		// set method definition and parent class
		element.setMethodDef(rootInfusion.getMethodDefinition(element.getMethodDefGlobalId()));
		element.setParentClass(rootInfusion.getClassDefinition(element.getParentClassGlobalId()));
		super.visit(element);
	}
	
	public void visit(ExternalMethod element)
	{
		element.setMethodDef(rootInfusion.getMethodDefinition(element.getMethodDefinitionid()));
		element.setMethodImpl(rootInfusion.getMethodImplementation(element.getMethodDefinitionid()));
	}
	
	@Override
	public void visit(ExternalField element)
	{
		element.setParentClass(rootInfusion.getClassDefinition(element.getParentClassGlobalId()));
	}
	
	public void visit(ExternalClassDefinition element)
	{
		for (AbstractField field : element.getFieldList().getFields())
			field.setParentClass(rootInfusion.getClassDefinition(((ExternalField)field).getParentClassGlobalId()));
		
		// call the parent visit function 
		super.visit(element);
	}
	
	@Override
	public void visit(Element element)
	{
	}	

}
