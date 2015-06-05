/*
 * ExternalPartialInfusion.java
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

import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;

import org.csiro.darjeeling.infuser.structure.GlobalId;
import org.csiro.darjeeling.infuser.structure.elements.AbstractClassDefinition;
import org.csiro.darjeeling.infuser.structure.elements.AbstractField;
import org.csiro.darjeeling.infuser.structure.elements.AbstractInfusion;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodDefinition;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodImplementation;
import org.w3c.dom.Node;

public class ExternalPartialInfusion extends AbstractInfusion
{
	
	public static ExternalPartialInfusion fromNode(Node node)
	{
		ExternalPartialInfusion ret = new ExternalPartialInfusion();
		
		XPath xpath = XPathFactory.newInstance().newXPath();
		try {
			
			// read Header
			Node headerNode = (Node)xpath.evaluate("header", node, XPathConstants.NODE);
			ret.setHeader(ExternalHeader.fromNode(headerNode));
			//System.out.println("ref: " + ret.getHeader().getInfusionName());
			
		} catch (XPathExpressionException ex)
		{
			throw new RuntimeException(ex);
		}

		/*
		// read class list
		ret.setClassList(ExternalClassList.fromDocument(doc));

		// read method definition list
		ret.setMethodDefinitionList(ExternalMethodDefinitionList.fromDocument(doc));

		// read method implementation list
		ret.setMethodImplementationList(ExternalMethodImplementationList.fromDocument(doc));

		// read static field list
		ret.setStaticFieldList(ExternalStaticFieldList.fromDocument(doc));
		
		// read string table
		ret.setStringTable(ExternalStringTable.fromDocument(doc));
		
		// read imported infusions
		 */
		

		return ret;
	}	
	public AbstractMethodImplementation lookupMethodImplemention(AbstractMethodDefinition methodDef, String className)
	{
		return null;
	}

	public AbstractMethodDefinition lookupMethodDefinition(AbstractMethodDefinition methodDef)
	{
		return null;
	}

	public AbstractClassDefinition lookupClassByName(String className)
	{
		return null;
	}

	public GlobalId lookupString(String str)
	{
		return null;
	}

	public AbstractField lookupStaticField(String className, String fieldName, String descriptor)
	{
		return null;
	}

}
