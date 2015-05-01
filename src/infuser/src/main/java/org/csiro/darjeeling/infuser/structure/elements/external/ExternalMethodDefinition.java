/*
 * ExternalMethodDefinition.java
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
 
package org.csiro.darjeeling.infuser.structure.elements.external;

import org.csiro.darjeeling.infuser.structure.ElementVisitor;
import org.csiro.darjeeling.infuser.structure.GlobalId;
import org.csiro.darjeeling.infuser.structure.elements.AbstractInfusion;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodDefinition;
import org.w3c.dom.Node;

public class ExternalMethodDefinition extends AbstractMethodDefinition
{
	

	public ExternalMethodDefinition(String methodName, String descriptor)
	{
		super(methodName, descriptor);
	}

	public static ExternalMethodDefinition fromNode(Node node, AbstractInfusion infusion)
	{
		ExternalMethodDefinition methodDef = new ExternalMethodDefinition(
				NodeUtil.getAttribute(node, "name"),
				NodeUtil.getAttribute(node, "signature")
				);
		
		if (methodDef.methodName==null) throw new HeaderParseException("Method name missing in method definition");
		if (methodDef.descriptor==null) throw new HeaderParseException("Method signature missing in method definition");
		
		String infusionName = infusion.getHeader().getInfusionName();
		int localId = NodeUtil.getIntAttribute(node, "entity_id");
		
		methodDef.setGlobalId(new GlobalId(infusionName, localId));
		
		return methodDef;
	}
	
	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}	
	
}
