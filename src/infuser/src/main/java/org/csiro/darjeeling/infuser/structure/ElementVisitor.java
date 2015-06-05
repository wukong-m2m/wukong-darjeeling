/*
 * ElementVisitor.java
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
 
package org.csiro.darjeeling.infuser.structure;

import org.csiro.darjeeling.infuser.structure.elements.AbstractClassDefinition;
import org.csiro.darjeeling.infuser.structure.elements.AbstractClassList;
import org.csiro.darjeeling.infuser.structure.elements.AbstractField;
import org.csiro.darjeeling.infuser.structure.elements.AbstractHeader;
import org.csiro.darjeeling.infuser.structure.elements.AbstractInfusion;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethod;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodDefinition;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodDefinitionList;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodImplementation;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodImplementationList;
import org.csiro.darjeeling.infuser.structure.elements.AbstractReferencedInfusionList;
import org.csiro.darjeeling.infuser.structure.elements.AbstractStaticFieldList;
import org.csiro.darjeeling.infuser.structure.elements.AbstractStringTable;
import org.csiro.darjeeling.infuser.structure.elements.external.ExternalClassDefinition;
import org.csiro.darjeeling.infuser.structure.elements.external.ExternalClassList;
import org.csiro.darjeeling.infuser.structure.elements.external.ExternalField;
import org.csiro.darjeeling.infuser.structure.elements.external.ExternalHeader;
import org.csiro.darjeeling.infuser.structure.elements.external.ExternalInfusion;
import org.csiro.darjeeling.infuser.structure.elements.external.ExternalMethod;
import org.csiro.darjeeling.infuser.structure.elements.external.ExternalMethodDefinition;
import org.csiro.darjeeling.infuser.structure.elements.external.ExternalMethodDefinitionList;
import org.csiro.darjeeling.infuser.structure.elements.external.ExternalMethodImplementation;
import org.csiro.darjeeling.infuser.structure.elements.external.ExternalMethodImplementationList;
import org.csiro.darjeeling.infuser.structure.elements.external.ExternalReferencedInfusionList;
import org.csiro.darjeeling.infuser.structure.elements.external.ExternalStaticFieldList;
import org.csiro.darjeeling.infuser.structure.elements.external.ExternalStringTable;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalClassDefinition;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalClassList;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalField;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalHeader;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalInfusion;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethod;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodDefinition;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodDefinitionList;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodImplementation;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodImplementationList;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalReferencedInfusionList;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalStaticFieldList;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalStringTable;

/**
 * Element visitors are used to traverse the element tree. 
 * 
 * @author Niels Brouwers
 *
 */
public abstract class ElementVisitor
{
	
	public void visit(InternalInfusion element)
	{
		visit((AbstractInfusion)element);
	}
	
	public void visit(ExternalInfusion element)
	{
		visit((AbstractInfusion)element);
	}
	
	public void visit(AbstractInfusion element)
	{
		visit((ParentElement<Element>)element);
	}
	
	public void visit(InternalStringTable element)
	{
		visit((AbstractStringTable)element);
	}

	public void visit(ExternalStringTable element)
	{
		visit((AbstractStringTable)element);
	}

	public void visit(AbstractStringTable element)
	{
		visit((Element)element);
	}

	public void visit(InternalHeader element)
	{
		visit((AbstractHeader)element);
	}

	public void visit(ExternalHeader element)
	{
		visit((AbstractHeader)element);
	}

	public void visit(AbstractHeader element)
	{
		visit((Element)element);
	}

	public void visit(InternalClassList element)
	{
		visit((AbstractClassList)element);
	}
	
	public void visit(ExternalClassList element)
	{
		visit((AbstractClassList)element);
	}
	
	public void visit(AbstractClassList element)
	{
		visit((ParentElement<AbstractClassDefinition>)element);
	}
	
	public void visit(InternalClassDefinition element)
	{
		visit((AbstractClassDefinition)element);
	}
	
	public void visit(ExternalClassDefinition element)
	{
		visit((AbstractClassDefinition)element);
	}
	
	public void visit(AbstractClassDefinition element)
	{
		visit((ParentElement<AbstractMethod>)element);
	}
	
	public void visit(InternalMethodDefinitionList element)
	{
		visit((AbstractMethodDefinitionList)element);
	}

	public void visit(ExternalMethodDefinitionList element)
	{
		visit((AbstractMethodDefinitionList)element);
	}

	public void visit(AbstractMethodDefinitionList element)
	{
		visit((ParentElement<AbstractMethodDefinition>)element);
	}

	public void visit(InternalMethodDefinition element)
	{
		visit((AbstractMethodDefinition)element);
	}
	
	public void visit(ExternalMethodDefinition element)
	{
		visit((AbstractMethodDefinition)element);
	}
	
	public void visit(AbstractMethodDefinition element)
	{
		visit((Element)element);
	}
	
	public void visit(InternalStaticFieldList element)
	{
		visit((AbstractStaticFieldList)element);
	}
	
	public void visit(ExternalStaticFieldList element)
	{
		visit((AbstractStaticFieldList)element);
	}
	
	public void visit(AbstractStaticFieldList element)
	{
		visit((ParentElement<AbstractField>)element);
	}
	
	public void visit(InternalField field)
	{
		visit((AbstractField)field);
	}
	
	public void visit(ExternalField field)
	{
		visit((AbstractField)field);
	}
	
	public void visit(AbstractField field)
	{
		visit((Element)field);
	}
	
	public void visit(InternalMethod element)
	{
		visit((AbstractMethod)element);
	}
	
	public void visit(ExternalMethod element)
	{
		visit((AbstractMethod)element);
	}
	
	public void visit(AbstractMethod element)
	{
		visit((Element)element);
	}
	
	public void visit(InternalMethodImplementation element)
	{
		visit((AbstractMethodImplementation)element);
	}
	
	public void visit(ExternalMethodImplementation element)
	{
		visit((AbstractMethodImplementation)element);
	}
	
	public void visit(AbstractMethodImplementation element)
	{
		visit((Element)element);
	}

	public void visit(InternalMethodImplementationList element)
	{
		visit((AbstractMethodImplementationList)element);
	}

	public void visit(ExternalMethodImplementationList element)
	{
		visit((AbstractMethodImplementationList)element);
	}

	public void visit(AbstractMethodImplementationList element)
	{
		visit((ParentElement<AbstractMethodImplementation>)element);
	}

	public void visit(InternalReferencedInfusionList element)
	{
		visit((AbstractReferencedInfusionList)element);
	}

	public void visit(ExternalReferencedInfusionList element)
	{
		visit((AbstractReferencedInfusionList)element);
	}

	public void visit(AbstractReferencedInfusionList element)
	{
		visit((ParentElement<AbstractInfusion>)element);
	}

	public <T extends Element> void visit(ParentElement<T> element)
	{
		visit((Element)element);
	}

	public <T extends Element> void visitChildren(ParentElement<T> element)
	{
		for (Element child : element.getChildren())
			child.accept(this);
	}
	
	public abstract void visit(Element element);

}
