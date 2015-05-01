/*
 * NodeUtil.java
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
import javax.xml.xpath.XPathExpression;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;

import org.csiro.darjeeling.infuser.structure.LocalId;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class NodeUtil
{
	
	private static XPathFactory factory;
	private static XPath xpath;
	
	static 
	{
		factory = XPathFactory.newInstance();
		xpath = factory.newXPath();
	}
	
	public static void ensureAttributes(Node node, String ... attributeNames) throws HeaderParseException
	{
		for (String attributeName : attributeNames)
			if (node.getAttributes().getNamedItem(attributeName)==null)
				throw new HeaderParseException(String.format("Attribute %s missing in node %s", attributeName, node.toString()));
	}
	
	public static String getAttribute(Node node, String attributeName)
	{
		Node attributeNode = node.getAttributes().getNamedItem(attributeName);
		
		if (attributeNode==null)
			return null;
		else
			return attributeNode.getNodeValue();
	}

	public static int getIntAttribute(Node node, String attributeName, int def)
	{
		int ret = def;
		String str = getAttribute(node, attributeName);
		if (str==null) return 0;
		
		try {
			ret = Integer.parseInt(str);
		} catch (NumberFormatException ex)
		{
			// do nothing, just pass out the default
		}
		
		return ret;
	}

	public static int getIntAttribute(Node node, String attributeName)
	{
		int ret;
		String str = getAttribute(node, attributeName);
		try {
			ret = Integer.parseInt(str);
		} catch (NumberFormatException ex)
		{
			throw new HeaderParseException(String.format("Error while parsing darjeeling header, '%s' is not a number", str));
		}

		return ret;
	}
	
	public static LocalId getGlobalId(Node node, String attributeName)
	{
		int infusionId = getIntAttribute(node, attributeName + ".infusion", -1);
		int localId = getIntAttribute(node, attributeName + ".index", -1);
		return new LocalId(infusionId, localId);
	}
	
	public static NodeList getNodesByXPath(Document doc, String expression)
	{
		try {
			XPathExpression expr = xpath.compile(expression);
			return (NodeList)expr.evaluate(doc, XPathConstants.NODESET);
		} catch (XPathExpressionException ex)
		{
			throw new HeaderParseException("Error while parsing darjeeling header, expression exception ocurred: " + ex.getMessage());
		}
	}
	
	public static Node getNodeByXPath(Document doc, String expression)
	{
		NodeList nodes = getNodesByXPath(doc, expression);
		if (nodes.getLength()==0) throw new HeaderParseException("Error while parsing darjeeling header, node " + expression + " not found");
		if (nodes.getLength()>1) throw new HeaderParseException("Error while parsing darjeeling header, multiple nodes found for " + expression + " where only one was expected");
		return nodes.item(0);
	}	
}
