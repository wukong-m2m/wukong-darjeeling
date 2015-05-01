/*
 * FieldMapVisitor.java
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
 
package org.csiro.darjeeling.infuser.processingphase;

import org.csiro.darjeeling.infuser.structure.DescendingVisitor;
import org.csiro.darjeeling.infuser.structure.Element;
import org.csiro.darjeeling.infuser.structure.elements.AbstractClassDefinition;
import org.csiro.darjeeling.infuser.structure.elements.AbstractField;
import org.csiro.darjeeling.infuser.structure.elements.AbstractStaticFieldList;

/**
 * The field map visitor assigns offsets to static and non-static fields. 
 * 
 * @author Niels Brouwers
 *
 */
public class FieldMapVisitor extends DescendingVisitor
{
	
	@Override
	/**
	 * Processes non-static fields.
	 */
	public void visit(AbstractClassDefinition element)
	{
		int nonRefOffset = 0;
		int refOffset = 0;
		
		AbstractClassDefinition superClass = element.getSuperClass();
		if (superClass!=null)
		{
			nonRefOffset += superClass.getNonReferenceFieldsSize();
			refOffset += superClass.getReferenceFieldCount();
		}
		
		for (AbstractField field : element.getFieldList().getFields())
		{
			if (field.isRef())
			{
				field.setOffset(refOffset);
				refOffset++;
			} else
			{
				field.setOffset(nonRefOffset);
				nonRefOffset += field.getSize();
			}
		}
	}
	
	@Override
	/**
	 * Processes static fields.
	 */
	public void visit(AbstractStaticFieldList element)
	{
		// TODO: use the BaseType enum here
		int counters[] = new int[5];
		
		for (AbstractField field : element.getChildren())
		{
			int counterIndex;
			switch (field.getSize())
			{
				case 1: counterIndex = 0; break;
				case 2: counterIndex = 1; break;
				case 4: counterIndex = 2; break;
				case 8: counterIndex = 3; break;
				case -1: counterIndex = 4; break;
				default:
					throw new IllegalStateException("Unhandled field size: " + field.getSize());
			}
			
			field.getGlobalId().setEntityId(counters[counterIndex]);
			counters[counterIndex]++;
		}
		
		element.setNrBytes(counters[0]);
		element.setNrShorts(counters[1]);
		element.setNrInts(counters[2]);
		element.setNrLongs(counters[3]);
		element.setNrRefs(counters[4]);
	
	}
	
	@Override
	public void visit(Element element)
	{
	}

}
