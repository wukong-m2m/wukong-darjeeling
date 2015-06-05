/*
 * ExternalField.java
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
import org.csiro.darjeeling.infuser.structure.Flags;
import org.csiro.darjeeling.infuser.structure.GlobalId;
import org.csiro.darjeeling.infuser.structure.elements.AbstractClassDefinition;
import org.csiro.darjeeling.infuser.structure.elements.AbstractField;
import org.w3c.dom.Node;

public class ExternalField extends AbstractField
{
	
	private GlobalId parentClassGlobalId;

	private ExternalField(String name, String descriptor, int size, Flags flags, AbstractClassDefinition parentClass)
	{
		super(name, descriptor, size, flags, parentClass);
	}
	
	public static ExternalField fromNode(Node node, ExternalInfusion infusion)
	{
		String name = NodeUtil.getAttribute(node, "name");
		String signature = NodeUtil.getAttribute(node, "signature");
		int size = classify(signature).getSize();
		
		ExternalField ret = new ExternalField(
				name,
				signature,
				size,
				new Flags(),
				null
				);
		
		int entityId = NodeUtil.getIntAttribute(node, "entity_id");
		ret.setGlobalID(new GlobalId(infusion.getHeader().getInfusionName(), entityId));

		String infusionName = NodeUtil.getAttribute(node, "parentclass.infusion");
		int localId = NodeUtil.getIntAttribute(node, "parentclass.entity_id");
		ret.parentClassGlobalId = new GlobalId(infusionName, localId);
		
		return ret;
	}
	
	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}
	
	public GlobalId getParentClassGlobalId()
	{
		return parentClassGlobalId;
	}
	
}
