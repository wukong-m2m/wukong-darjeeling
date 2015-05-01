/*
 * ExternalMethodImplementation.java
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
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodImplementation;
import org.w3c.dom.Node;

public class ExternalMethodImplementation extends AbstractMethodImplementation
{
	
	private GlobalId methodDefGlobalId, parentClassGlobalId;

	protected ExternalMethodImplementation()
	{
	}
	
	public static ExternalMethodImplementation fromNode(Node node, ExternalInfusion infusion, String fileName)
	{
		// check if the node has the required attributes
		if (NodeUtil.getAttribute(node, "entity_id")==null) throw new HeaderParseException("entity_id attribute missing in method implementation");
		if (NodeUtil.getAttribute(node, "integer_argument_count")==null) throw new HeaderParseException("integer_argument_count attribute missing in method implementation");
		if (NodeUtil.getAttribute(node, "reference_argument_count")==null) throw new HeaderParseException("reference_argument_count attribute missing in method implementation");
		if (NodeUtil.getAttribute(node, "methoddef.infusion")==null) throw new HeaderParseException("methoddef.infusion attribute missing in method implementation");
		if (NodeUtil.getAttribute(node, "methoddef.entity_id")==null) throw new HeaderParseException("methoddef.entity_id attribute missing in method implementation");
		if (NodeUtil.getAttribute(node, "parentclass.infusion")==null) throw new HeaderParseException("methoddef.infusion attribute missing in method implementation");
		if (NodeUtil.getAttribute(node, "parentclass.entity_id")==null) throw new HeaderParseException("methoddef.entity_id attribute missing in method implementation");
		
		// create the method implementation object
		ExternalMethodImplementation ret = new ExternalMethodImplementation();
		ret.fileName = fileName;
		
		String infusionName = infusion.getHeader().getInfusionName();
		int localId = NodeUtil.getIntAttribute(node, "entity_id");
		ret.setGlobalId(new GlobalId(infusionName, localId));

		ret.integerArgumentCount = NodeUtil.getIntAttribute(node, "integer_argument_count");
		ret.referenceArgumentCount = NodeUtil.getIntAttribute(node, "reference_argument_count");

		infusionName = NodeUtil.getAttribute(node, "methoddef.infusion");
		localId = NodeUtil.getIntAttribute(node, "methoddef.entity_id");
		ret.methodDefGlobalId = new GlobalId(infusionName, localId);

		infusionName = NodeUtil.getAttribute(node, "parentclass.infusion");
		localId = NodeUtil.getIntAttribute(node, "parentclass.entity_id");
		ret.parentClassGlobalId = new GlobalId(infusionName, localId);

		return ret;
	}	
	
	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}

	@Override
	public GlobalId getMethodDefGlobalId()
	{
		return methodDefGlobalId;
	}

	@Override
	public GlobalId getParentClassGlobalId()
	{
		return parentClassGlobalId;
	}


}
