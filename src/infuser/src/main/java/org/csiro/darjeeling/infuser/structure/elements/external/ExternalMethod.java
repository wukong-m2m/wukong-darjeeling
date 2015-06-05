/*
 * ExternalMethod.java
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
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethod;
import org.w3c.dom.Node;

public class ExternalMethod extends AbstractMethod
{
	
	private GlobalId methodDefinitionid, methodImplementationId;
	
	public static ExternalMethod fromNode(Node node, AbstractInfusion infusion)
	{
		// check if the node has the required attributes
		if (NodeUtil.getAttribute(node, "methoddef.infusion")==null) throw new HeaderParseException("methoddef.infusion attribute missing in method");
		if (NodeUtil.getAttribute(node, "methoddef.entity_id")==null) throw new HeaderParseException("methoddef.entity_id attribute missing in method");
		if (NodeUtil.getAttribute(node, "methodimpl.infusion")==null) throw new HeaderParseException("methodimpl.infusion attribute missing in method");
		if (NodeUtil.getAttribute(node, "methodimpl.entity_id")==null) throw new HeaderParseException("methodimpl.entity_id attribute missing in method");
		
		// create the method implementation object
		ExternalMethod ret = new ExternalMethod();
		
		String infusionName = NodeUtil.getAttribute(node, "methoddef.infusion");
		int localId = NodeUtil.getIntAttribute(node, "methoddef.entity_id");
		ret.methodDefinitionid = new GlobalId(infusionName, localId);

		infusionName = NodeUtil.getAttribute(node, "methodimpl.infusion");
		localId = NodeUtil.getIntAttribute(node, "methodimpl.entity_id");
		ret.methodImplementationId = new GlobalId(infusionName, localId);
		
		return ret;
	}
	
	public GlobalId getMethodDefinitionid()
	{
		return methodDefinitionid;
	}
	
	public GlobalId getMethodImplementationId()
	{
		return methodImplementationId;
	}

	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}

}
