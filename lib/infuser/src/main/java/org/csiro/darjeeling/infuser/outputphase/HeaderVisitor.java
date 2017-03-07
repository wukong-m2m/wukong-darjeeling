/*
 * HeaderVisitor.java
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
 
package org.csiro.darjeeling.infuser.outputphase;

import org.apache.bcel.generic.Type;
import org.csiro.darjeeling.infuser.Constants;
import org.csiro.darjeeling.infuser.structure.BaseType;
import org.csiro.darjeeling.infuser.structure.DescendingVisitor;
import org.csiro.darjeeling.infuser.structure.Element;
import org.csiro.darjeeling.infuser.structure.ParentElement;
import org.csiro.darjeeling.infuser.structure.elements.AbstractClassDefinition;
import org.csiro.darjeeling.infuser.structure.elements.AbstractField;
import org.csiro.darjeeling.infuser.structure.elements.AbstractHeader;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethod;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodDefinition;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodImplementation;
import org.csiro.darjeeling.infuser.structure.elements.AbstractStringTable;
import org.csiro.darjeeling.infuser.structure.elements.external.ExternalReferencedInfusionList;
import org.w3c.dom.Document;

/**
 * 
 * This visitor exports the information tree as an infusion header (.DIH) file. These files are in XML format, so an org.w3c.dom.Document is built up and written 
 * out later on.
 * 
 * @author Niels Brouwers
 *
 */
public class HeaderVisitor extends DescendingVisitor
{
	
	// output XML document
	private Document doc;
	
	// the current parent element the visitor is 'in'
	private org.w3c.dom.Element currentElement;
	
	/**
	 * Creates a new HeaderVisitor object.
	 * @param doc a constructed XML document to write to
	 */
	public HeaderVisitor(Document doc)
	{
		this.doc = doc;
		
		// create root node
		currentElement = doc.createElement("dih");
		doc.appendChild(currentElement);
	}
	
	/**
	 * creates an XML element from an org.csiro.darjeeling.infuser.structure.Element and appends it to the current element. 
	 * It uses the element name as the XML node name.
	 * @param element the element to create a new XML node for
	 * @return the newly created XML node
	 */
	private org.w3c.dom.Element createElement(Element element)
	{
		org.w3c.dom.Element elem = doc.createElement(getElementName(element).toLowerCase());
		currentElement.appendChild(elem);
		return(elem);
	}

	/**
	 * Creates a new XML node for a parent element
	 * @param element and Element object
	 */
	public <T extends Element> void visit(ParentElement<T> element)
	{
		// create XML element for this node
		org.w3c.dom.Element elem = createElement(element);
		
		// visit children
		org.w3c.dom.Element temp = currentElement;
		currentElement = elem;
		
		super.visit(element);
		
		currentElement = temp;
		
	}
	
	/**
	 * Creates an XML representation of an AbstractClassDefinition.
	 * Attributes are entity_id and name, as well as superclass.infusion, superclass.entity_id and superclass.name. If the class definition
	 * is for java.lang.Object, the superclass entity_id is set to -1 to indicate that there is no superclass.
	 * <p>
	 * Child nodes of this class are of node type <b>field</b> and <b>method</b>.
	 */
	public void visit(AbstractClassDefinition element)
	{
		// create XML element for this node
		org.w3c.dom.Element elem = createElement(element);
		
		elem.setAttribute("entity_id", "" + element.getGlobalId().getEntityId());
		elem.setAttribute("name", "" + element.getName());
		
		// at this point, the only reason why superClass can be null is when
		// infusing the system library, because the java.lang.Object class cannot
		// have a super class. Javac will set java.lang.Object as the superclass
		// of java.lang.Object, but that would create a circular reference so we
		// just output (sys,-1) instead. This is safe, since indices are always 
		// positive, and it can be checked quite easily. Not very pretty though :/
		AbstractClassDefinition superClass = element.getSuperClass();
		if (superClass!=null)
		{
			elem.setAttribute("superclass.infusion", "" + element.getSuperClass().getGlobalId().getInfusion());
			elem.setAttribute("superclass.entity_id", "" + element.getSuperClass().getGlobalId().getEntityId());
			elem.setAttribute("superclass.name", superClass.getName());
		} else
		{
			elem.setAttribute("superclass.infusion", "sys");
			elem.setAttribute("superclass.entity_id", "-1");
			elem.setAttribute("superclass.name", "");
		}

		org.w3c.dom.Element temp = currentElement;
		currentElement = elem;
		
		// write fields
		for (AbstractField field : element.getFieldList().getFields())
		{
			field.accept(this);
		}

		// visit children
		for (AbstractMethod method : element.getChildren())
		{
			method.accept(this);
		}
		
		currentElement = temp;

	}
	
	/**
	 * Creates an XML representation of an AbstractMethod element.
	 */
	public void visit(AbstractMethod element)
	{
		// create XML element for this node
		org.w3c.dom.Element elem = createElement(element);

		elem.setAttribute("methodimpl.infusion", element.getMethodImpl().getGlobalId().getInfusion());
		elem.setAttribute("methodimpl.entity_id", "" + element.getMethodImpl().getGlobalId().getEntityId());
		elem.setAttribute("methoddef.infusion", element.getMethodDef().getGlobalId().getInfusion());
		elem.setAttribute("methoddef.entity_id", "" + element.getMethodDef().getGlobalId().getEntityId());
	}
	
	/**
	 * Export the static field definition.
	 */
	@Override
	public void visit(AbstractField field)
	{
		org.w3c.dom.Element elem = createElement(field);
		Type type = Type.getType(field.getDescriptor());
		
		elem.setAttribute("entity_id", ""+field.getGlobalId().getEntityId());
		elem.setAttribute("type", ""+BaseType.fromBCELType(type));
		elem.setAttribute("name", field.getName());
		elem.setAttribute("signature", field.getDescriptor());

		elem.setAttribute("parentclass.infusion", field.getParentClass().getGlobalId().getInfusion());
		elem.setAttribute("parentclass.entity_id", "" + field.getParentClass().getGlobalId().getEntityId());
		
	}
	
	@Override
	public void visit(AbstractStringTable stringTable)
	{
		org.w3c.dom.Element elem = createElement(stringTable);
		
		String strings[] = stringTable.elements();
		for (int i=0; i<strings.length; i++)
		{
			org.w3c.dom.Element childElem = doc.createElement("string");
			childElem.setAttribute("entity_id", "" + i);
			childElem.setAttribute("value", strings[i]);
			elem.appendChild(childElem);
		}
	}
	
	@Override
	public void visit(AbstractHeader element)
	{
		org.w3c.dom.Element elem = createElement(element);
		elem.setAttribute("infusionformatversion", "" + element.getInfusionFormatVersion());
		elem.setAttribute("infusionversion", "" + element.getInfusionVersion());
		elem.setAttribute("name", element.getInfusionName());
		
		int entryPoint = Constants.NO_ENTRYPOINT;
		if (element.getEntryPoint()!=null) entryPoint = element.getEntryPoint().getGlobalId().getEntityId();
		elem.setAttribute("entrypoint", "" + entryPoint);
	}
	
	@Override
	public void visit(AbstractMethodImplementation element)
	{
		org.w3c.dom.Element elem = createElement(element);
		elem.setAttribute("entity_id", "" + element.getGlobalId().getEntityId());
		
		elem.setAttribute("integer_argument_count", "" + element.getIntegerArgumentCount());
		elem.setAttribute("reference_argument_count", "" + element.getReferenceArgumentCount());
		
		elem.setAttribute("methoddef.infusion", "" + element.getMethodDefinition().getGlobalId().getInfusion());
		elem.setAttribute("methoddef.entity_id", "" + element.getMethodDefinition().getGlobalId().getEntityId());
		
		elem.setAttribute("parentclass.infusion", "" + element.getParentClass().getGlobalId().getInfusion());
		elem.setAttribute("parentclass.entity_id", "" + element.getParentClass().getGlobalId().getEntityId());
	}
	
	@Override
	public void visit(AbstractMethodDefinition element)
	{
		org.w3c.dom.Element elem = createElement(element);
		elem.setAttribute("signature", element.getSignature());
		elem.setAttribute("name", element.getName());
		elem.setAttribute("entity_id", "" + element.getGlobalId().getEntityId());
	}
	
	/**
	 * Write out a list of infusions referenced by this one. Since this is an external referenced infusion list,
	 * only write out the headers.
	 */
	@Override
	public void visit(ExternalReferencedInfusionList element)
	{
		super.visit(element);
	}
	
	@Override
	public void visit(Element element)
	{
		
		org.w3c.dom.Element elem = doc.createElement(getElementName(element));
		currentElement.appendChild(elem);
		
	}
	
	private String getElementName(Element element)
	{
		return element.getId().toString();
	}

}
