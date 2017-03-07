/*
 * AbstractField.java
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

import org.csiro.darjeeling.infuser.structure.BaseType;
import org.csiro.darjeeling.infuser.structure.Element;
import org.csiro.darjeeling.infuser.structure.ElementId;
import org.csiro.darjeeling.infuser.structure.ElementVisitor;
import org.csiro.darjeeling.infuser.structure.Flags;
import org.csiro.darjeeling.infuser.structure.GlobalId;

/**
 * TODO refactor this whole thing
 * 
 * @author Niels brouwers
 *
 */
public class AbstractField extends Element
{
	
	protected String name, descriptor;
	protected int size;
	protected int offset;
	protected Flags flags;
	protected AbstractClassDefinition parentClass;
	protected GlobalId globalId;
	protected Object constantValue;
	
	protected AbstractField(String name, String descriptor, int size, Flags flags, AbstractClassDefinition parentClass)
	{
		super(ElementId.FIELDDEF);
		this.name = name;
		this.descriptor = descriptor;
		this.size = size;
		this.offset = 0;
		this.flags = flags;
		this.parentClass = parentClass;
	}
	
	public BaseType classify()
	{
		return AbstractField.classify(descriptor);
	}
	
	/**
	 * Returns the size of a base type, or -1 if it's a reference type
	 * @param descriptor java field descriptor, check the JVM spec ver2, 
	 * Section 4.3.2 for an overview 
	 */
	public static BaseType classify(String descriptor)
	{
		
		// descriptors starting with '[' are arrays 
		if (descriptor.startsWith("["))
			return BaseType.Ref;

		// descriptors starting with 'L' are objects 
		if (descriptor.startsWith("L"))
			return BaseType.Ref;
		
		// byte
		if (descriptor.startsWith("B"))
			return BaseType.Byte;

		// char
		if (descriptor.startsWith("C"))
			return BaseType.Char;

		// boolean
		if (descriptor.startsWith("Z"))
			return BaseType.Boolean;

		// short
		if (descriptor.startsWith("S"))
			return BaseType.Short;
		
		// int
		if (descriptor.startsWith("I"))
			return BaseType.Int;

		// float
		if (descriptor.startsWith("F"))
			return BaseType.Float;

		// double, long
		if (descriptor.startsWith("J"))
			return BaseType.Long;
		
		// double, long
		if (descriptor.startsWith("D"))
			return BaseType.Double;
		
		throw new IllegalStateException("error getting size for field descriptor: " + descriptor);
		
	}

	/**
	 * @return the offset
	 */
	public GlobalId getGlobalId()
	{
		return globalId;
	}

	/**
	 * @param offset the offset to set
	 */
	public void setGlobalID(GlobalId globalId)
	{
		this.globalId = globalId;
	}

	/**
	 * @return the name
	 */
	public String getName()
	{
		return name;
	}

	/**
	 * @return the descriptor
	 */
	public String getDescriptor()
	{
		return descriptor;
	}

	/**
	 * @return the size
	 */
	public int getSize()
	{
		return size;
	}

	/**
	 * @return the isRef
	 */
	public boolean isRef()
	{
		return size==-1;
	}
	
	/**
	 * @return the flags
	 */
	public Flags getFlags()
	{
		return flags;
	}
	
	public void setConstantValue(Object constantValue)
	{
		this.constantValue = constantValue;
	}
	
	public Object getConstantValue()
	{
		return constantValue;
	}
	
	/**
	 * @return the parentClass
	 */
	public AbstractClassDefinition getParentClass()
	{
		return parentClass;
	}

	public void setParentClass(AbstractClassDefinition parentClass)
	{
		this.parentClass = parentClass;
	}
	
	public void setOffset(int offset)
	{
		this.offset = offset;
	}
	
	public int getOffset()
	{
		return offset;
	}
	
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}
	
}
