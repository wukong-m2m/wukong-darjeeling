/*
 * ExternalStaticFieldList.java
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
import org.csiro.darjeeling.infuser.structure.elements.AbstractStaticFieldList;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;

public class ExternalStaticFieldList extends AbstractStaticFieldList
{
	
	public static AbstractStaticFieldList fromDocument(Document doc, ExternalInfusion infusion)
	{
		AbstractStaticFieldList staticFieldList = new ExternalStaticFieldList();
		NodeList fieldDefNodes = NodeUtil.getNodesByXPath(doc, "dih/infusion/staticfieldlist/fielddef");
		
		for (int i=0; i<fieldDefNodes.getLength(); i++)
			staticFieldList.add(ExternalField.fromNode(fieldDefNodes.item(i), infusion));
		
		return staticFieldList;
	}	

	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}	
}

