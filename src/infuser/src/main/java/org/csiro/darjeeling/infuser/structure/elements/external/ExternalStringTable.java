/*
 * ExternalStringTable.java
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
import org.csiro.darjeeling.infuser.structure.elements.AbstractStringTable;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class ExternalStringTable extends AbstractStringTable
{
	
	private ExternalStringTable(ExternalInfusion infusion)
	{
		super(infusion);
	}
	
	public static ExternalStringTable fromDocument(Document doc, ExternalInfusion infusion)
	{
		ExternalStringTable ret = new ExternalStringTable(infusion);
		NodeList nodes = NodeUtil.getNodesByXPath(doc, "dih/infusion/stringtable/*");
		
		for (int i=0; i<nodes.getLength(); i++)
		{
			Node node = nodes.item(i);
			String str = node.getAttributes().getNamedItem("value").getNodeValue();
			ret.addString(str);
		}
		
		return ret;
		
	}
	
	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}
	
}
