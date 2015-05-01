/*
 * AbstractMethod.java
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

public class AbstractMethod extends Element
{
	
	private AbstractMethodDefinition methodDef;
	private AbstractMethodImplementation methodImpl;

	protected AbstractMethod(AbstractMethodDefinition methodDef, AbstractMethodImplementation methodImpl)
	{
		this();
		this.methodDef = methodDef;
		this.methodImpl = methodImpl;
	}

	protected AbstractMethod()
	{
		super(ElementId.METHOD);
	}

	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}

	/**
	 * @return the methodDef
	 */
	public AbstractMethodDefinition getMethodDef()
	{
		return methodDef;
	}

	/**
	 * @return the methodImpl
	 */
	public AbstractMethodImplementation getMethodImpl()
	{
		return methodImpl;
	}

	/**
	 * @param methodDef the methodDef to set
	 */
	public void setMethodDef(AbstractMethodDefinition methodDef)
	{
		this.methodDef = methodDef;
	}
	
	public void setMethodImpl(AbstractMethodImplementation methodImpl)
	{
		this.methodImpl = methodImpl;
	}
	
	

}
