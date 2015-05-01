/*
 * InternalField.java
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
 
package org.csiro.darjeeling.infuser.structure.elements.internal;

import org.apache.bcel.classfile.Constant;
import org.apache.bcel.classfile.ConstantInteger;
import org.apache.bcel.classfile.ConstantLong;
import org.apache.bcel.classfile.ConstantString;
import org.apache.bcel.classfile.ConstantValue;
import org.csiro.darjeeling.infuser.structure.ElementVisitor;
import org.csiro.darjeeling.infuser.structure.Flags;
import org.csiro.darjeeling.infuser.structure.GlobalId;
import org.csiro.darjeeling.infuser.structure.elements.AbstractClassDefinition;
import org.csiro.darjeeling.infuser.structure.elements.AbstractField;

/**
 * 
 * Represents a field inside and output infusion.  
 * 
 * @author Niels Brouwers
 *
 */
public class InternalField extends AbstractField
{

	/**
	 * Constructs a new InternalField instance. 
	 * @param name field name
	 * @param descriptor field descriptor (refer to the JVM specification for a detailed explanation on descriptors)
	 * @param size size in bytes
	 * @param flags flags as a Flags object
	 * @param parentClass java class the field is a member of 
	 */
	private InternalField(String name, String descriptor, int size, Flags flags, AbstractClassDefinition parentClass)
	{
		super(name, descriptor, size, flags, parentClass);
	}
	
	/**
	 * Constructs a new InternalField instance from a BCEL Field object. 
	 * @param field the BCEL Field object
	 * @param parentClass java class the field is a member of
	 * @param infusion infusion the field belongs to
	 * @return a new InternalField instance
	 */
	public static InternalField fromField(org.apache.bcel.classfile.Field field, AbstractClassDefinition parentClass, InternalInfusion infusion)
	{
		InternalField ret;
		String signature = field.getSignature();
		int size = classify(signature).getSize();
		
		ret = new InternalField(
				field.getName(),
				field.getSignature(),
				size,
				Flags.fromAcessFlags(field),
				parentClass
				);
		
		ConstantValue constant = field.getConstantValue();
		if (constant!=null)
		{
			Constant value = constant.getConstantPool().getConstant(constant.getConstantValueIndex());
			if (value instanceof ConstantInteger) ret.setConstantValue(((ConstantInteger)value).getBytes());
			if (value instanceof ConstantLong) ret.setConstantValue(((ConstantLong)value).getBytes());
			if (value instanceof ConstantString) ret.setConstantValue(((ConstantString)value).getStringIndex());
		}

		ret.setGlobalID(new GlobalId(infusion.getHeader().getInfusionName(),0));
		
		return ret;		
	}	

	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}
	
}
