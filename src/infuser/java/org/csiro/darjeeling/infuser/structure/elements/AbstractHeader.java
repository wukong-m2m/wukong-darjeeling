/*
 * AbstractHeader.java
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

import org.csiro.darjeeling.infuser.structure.Element;
import org.csiro.darjeeling.infuser.structure.ElementId;
import org.csiro.darjeeling.infuser.structure.ElementVisitor;

public class AbstractHeader extends Element
{
	
	private String infusionName;
	private int infusionVersion;
	private int infusionFormatVersion;
	private AbstractMethodImplementation entryPoint;
	
	protected AbstractHeader(String infusionName, int infusionFormatVersion, int infusionVersion)
	{
		super(ElementId.HEADER);
		this.infusionName = infusionName;
		this.infusionFormatVersion = infusionFormatVersion;
		this.infusionVersion = infusionVersion;
	}
	
	/**
	 * @return the infusionName
	 */
	public String getInfusionName()
	{
		return infusionName;
	}

	/**
	 * @return the infusionFormatVersion
	 */
	public int getInfusionFormatVersion()
	{
		return infusionFormatVersion;
	}

	/**
	 * @return the infusionVersion
	 */
	public int getInfusionVersion()
	{
		return infusionVersion;
	}
	
	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}

	/**
	 * @return the entryPoint
	 */
	public AbstractMethodImplementation getEntryPoint()
	{
		return entryPoint;
	}

	/**
	 * @param entryPoint the entryPoint to set
	 */
	public void setEntryPoint(AbstractMethodImplementation entryPoint)
	{
		this.entryPoint = entryPoint;
	}
}
