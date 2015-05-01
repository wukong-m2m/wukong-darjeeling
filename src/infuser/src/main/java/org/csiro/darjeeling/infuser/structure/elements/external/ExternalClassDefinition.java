/*
 * ExternalClassDefinition.java
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
import org.csiro.darjeeling.infuser.structure.elements.AbstractClassDefinition;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class ExternalClassDefinition extends AbstractClassDefinition
{
	
	public static ExternalClassDefinition fromNode(Node node, ExternalInfusion infusion, String fileName)
	{
		// make sure the required attributes are there
		NodeUtil.ensureAttributes(node, "name", "superclass.name", "entity_id"); 		
		
		ExternalClassDefinition classDef = new ExternalClassDefinition();
		
		classDef.className = NodeUtil.getAttribute(node, "name");
		classDef.superClassName = NodeUtil.getAttribute(node, "superclass.name");
		classDef.fileName = fileName;
		
		String infusionName = infusion.getHeader().getInfusionName();
		int localId = NodeUtil.getIntAttribute(node, "entity_id");
		
		classDef.setGlobalId(new GlobalId(infusionName, localId));
		
		// load fields, methods
		NodeList nodes = node.getChildNodes();
		for (int i=0; i<nodes.getLength(); i++)
		{
			Node childNode = nodes.item(i);
			if (childNode.getNodeName().equalsIgnoreCase("fielddef"))
			{
				 ExternalField field = ExternalField.fromNode(childNode, infusion);
				 classDef.fieldList.addField(field);
			}

			if (childNode.getNodeName().equalsIgnoreCase("method"))
			{
				 ExternalMethod method = ExternalMethod.fromNode(childNode, infusion);
				 classDef.add(method);
			}
		}
		
		return classDef;
	}
	
	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}

}
