/*
 * AbstractStaticFieldList.java
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
 
package org.csiro.darjeeling.infuser.structure.elements;

import org.csiro.darjeeling.infuser.structure.ElementId;
import org.csiro.darjeeling.infuser.structure.ElementVisitor;
import org.csiro.darjeeling.infuser.structure.ParentElement;

public class AbstractStaticFieldList extends ParentElement<AbstractField>
{
	
	protected int nrBytes;
	protected int nrShorts;
	protected int nrInts;
	protected int nrLongs;
	protected int nrRefs;
	
	protected AbstractStaticFieldList()
	{
		super(ElementId.STATICFIELDLIST);
	}
	
	public AbstractField findField(String className, String fieldName, String descriptor)
	{
		for (AbstractField field : getChildren())
			if ( (field.getParentClass().getName().equals(className)) &&
					(field.getName().equals(fieldName)) &&
					(field.getDescriptor().equals(descriptor)) )
				return field;
		
		// not found
		return null;
	}

	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}

	/**
	 * @return the nrBytes
	 */
	public int getNrBytes()
	{
		return nrBytes;
	}

	/**
	 * @param nrBytes the nrBytes to set
	 */
	public void setNrBytes(int nrBytes)
	{
		this.nrBytes = nrBytes;
	}

	/**
	 * @return the nrShorts
	 */
	public int getNrShorts()
	{
		return nrShorts;
	}

	/**
	 * @param nrShorts the nrShorts to set
	 */
	public void setNrShorts(int nrShorts)
	{
		this.nrShorts = nrShorts;
	}

	/**
	 * @return the nrInts
	 */
	public int getNrInts()
	{
		return nrInts;
	}

	/**
	 * @param nrInts the nrInts to set
	 */
	public void setNrInts(int nrInts)
	{
		this.nrInts = nrInts;
	}
	
	public void setNrLongs(int nrLongs)
	{
		this.nrLongs = nrLongs;
	}
	
	public int getNrLongs()
	{
		return nrLongs;
	}

	/**
	 * @return the nrRefs
	 */
	public int getNrRefs()
	{
		return nrRefs;
	}

	/**
	 * @param nrRefs the nrRefs to set
	 */
	public void setNrRefs(int nrRefs)
	{
		this.nrRefs = nrRefs;
	}
	
		

}
