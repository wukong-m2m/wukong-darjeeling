/*
 * IndexVisitor.java
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
import org.csiro.darjeeling.infuser.structure.GlobalId;
import org.csiro.darjeeling.infuser.structure.elements.AbstractClassDefinition;
import org.csiro.darjeeling.infuser.structure.elements.AbstractInfusion;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodDefinition;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodImplementation;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalClassList;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodDefinitionList;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodImplementationList;

public class IndexVisitor extends DescendingVisitor
{
	
	String infusionName;
	
	public IndexVisitor(AbstractInfusion infusion)
	{
		this.infusionName = infusion.getHeader().getInfusionName();		
	}
	
	public void visit(InternalClassList element)
	{
		int i=0;
		for (AbstractClassDefinition classDef : element.getChildren())
		{
			classDef.setGlobalId(new GlobalId(infusionName, i));
			i++;
		}
	}

	public void visit(InternalMethodDefinitionList element)
	{
		int i=0;
		for (AbstractMethodDefinition methodDef : element.getChildren())
		{
			methodDef.setGlobalId(new GlobalId(infusionName, i));
			i++;
		}
	}

	public void visit(InternalMethodImplementationList element)
	{
		int i=0;
		for (AbstractMethodImplementation methodImplementation : element.getChildren())
		{
			methodImplementation.setGlobalId(new GlobalId(infusionName, i));
			i++;
		}
	}
	
	@Override
	public void visit(Element element)
	{
	}

}
