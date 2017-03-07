/*
 * ExternalHeader.java
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
import org.csiro.darjeeling.infuser.structure.elements.AbstractHeader;
import org.w3c.dom.Document;
import org.w3c.dom.Node;

public class ExternalHeader extends AbstractHeader
{

	private ExternalHeader(String infusionName, int infusionFormatVersion, int infusionVersion)
	{
		super(infusionName, infusionFormatVersion, infusionFormatVersion);
	}
	
	public static ExternalHeader fromDocument(Document doc)
	{
		return fromNode(NodeUtil.getNodeByXPath(doc, "dih/infusion/header"));
	}

	public static ExternalHeader fromNode(Node node)
	{
		int infusionFormatVersion = Integer.parseInt(node.getAttributes().getNamedItem("infusionformatversion").getNodeValue());
		int infusionVersion = Integer.parseInt(node.getAttributes().getNamedItem("infusionversion").getNodeValue());
		String name = node.getAttributes().getNamedItem("name").getNodeValue();
		return new ExternalHeader(name, infusionFormatVersion, infusionVersion);
	}
	
	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}	

}
