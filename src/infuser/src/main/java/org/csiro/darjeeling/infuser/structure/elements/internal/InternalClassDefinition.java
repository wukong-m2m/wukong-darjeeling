/*
 * InternalClassDefinition.java
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

import org.apache.bcel.classfile.JavaClass;
import org.csiro.darjeeling.infuser.structure.ElementVisitor;
import org.csiro.darjeeling.infuser.structure.GlobalId;
import org.csiro.darjeeling.infuser.structure.elements.AbstractClassDefinition;

public class InternalClassDefinition extends AbstractClassDefinition
{
	
	/* bcel javaClass object. This value is assigned when a class definition is constructed a .class file, 
	 * and is used to obtain a ConstantPool object for bytecode analysis.
	 */
	private JavaClass javaClass;
	
	private GlobalId nameId;
	
	// the <CINIT> method that initialises a class at infusion load time.
	protected InternalMethodImplementation cInit;
	
	protected InternalClassDefinition()
	{
	}

	public static InternalClassDefinition fromJavaClass(JavaClass javaClass, InternalInfusion infusion)
	{
		InternalClassDefinition classDef = new InternalClassDefinition();
		
		classDef.javaClass = javaClass;
		classDef.fileName = javaClass.getSourceFileName();
		
		// get name, superclass name and flags 
		classDef.className = javaClass.getClassName();
		classDef.superClassName = javaClass.getSuperclassName();
		
		// get implemented interfaces
		for (String interfaceName : javaClass.getInterfaceNames())
			classDef.interfaces.put(interfaceName, null);			

		// if the class is not an interface, fields and methods
		if (!javaClass.isInterface())
		{
			// read fields
			for (org.apache.bcel.classfile.Field field : javaClass.getFields())
				if (!field.isStatic())
					classDef.fieldList.addField(InternalField.fromField(field, classDef, infusion));
		}
		
		return classDef;
	}
	
	/**
	 * @return the javaClass
	 */
	public JavaClass getJavaClass()
	{
		return javaClass;
	}
	
	public InternalMethodImplementation getCInit()
	{
		return cInit;
	}
	
	public void setCInit(InternalMethodImplementation cinit)
	{
		this.cInit = cinit;
	}
	
	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}
	
	public void setNameId(GlobalId nameId)
	{
		this.nameId = nameId;
	}
	
	public GlobalId getNameId()
	{
		return nameId;
	}

}
