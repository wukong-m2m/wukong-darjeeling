/*
 * AbstractMethodImplementation.java
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
import org.csiro.darjeeling.infuser.structure.GlobalId;

public abstract class AbstractMethodImplementation extends Element
{

	protected AbstractClassDefinition parentClass;
	protected AbstractMethodDefinition methodDefinition;
	protected GlobalId globalId;
	protected int integerArgumentCount, referenceArgumentCount;
	protected boolean isStatic;

	/* for error reporting */
	protected String fileName;

	protected AbstractMethodImplementation()
	{
		super(ElementId.METHODIMPL);
	}

	/**
	 * @return the parentClass
	 */
	public AbstractClassDefinition getParentClass()
	{
		return parentClass;
	}

	/**
	 * @return the methodDef
	 */
	public AbstractMethodDefinition getMethodDefinition()
	{
		return methodDefinition;
	}
	
	/**
	 * @param methodDef the methodDef to set
	 */
	public void setMethodDef(AbstractMethodDefinition methodDef)
	{
		this.methodDefinition = methodDef;
	}

	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}

	/**
	 * @return the localMethodImplId
	 */
	public GlobalId getGlobalId()
	{
		return this.globalId;
	}
	
	/**
	 * @param localMethodImplId the localMethodImplId to set
	 */
	public void setGlobalId(GlobalId globalId)
	{
		this.globalId = globalId;
	}

	public int getReferenceArgumentCount()
	{
		return referenceArgumentCount;
	}

	public int getIntegerArgumentCount()
	{
		return integerArgumentCount;
	}
	
	/**
	 * @return the methodDefGlobalId
	 */
	public abstract GlobalId getMethodDefGlobalId();
	
	public abstract GlobalId getParentClassGlobalId();

	/**
	 * @param parentClass the parentClass to set
	 */
	public void setParentClass(AbstractClassDefinition parentClass)
	{
		this.parentClass = parentClass;
	}
	
	public String getFileName()
	{
		return fileName;
	}
	
	public boolean isStatic()
	{
		return isStatic;
	}
	
	public void setStatic(boolean isStatic)
	{
		this.isStatic = isStatic;
	}
	
	@Override
	public String toString()
	{
		return String.format("MethodImplementation[%s:%s.%s:%s]",
				fileName,
				parentClass.getName(),
				methodDefinition.getName(),				
				methodDefinition.getSignature()				
				);
	}
	
}
