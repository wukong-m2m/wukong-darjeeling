/*
 * InternalInfusion.java
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
import org.csiro.darjeeling.infuser.structure.elements.AbstractField;
import org.csiro.darjeeling.infuser.structure.elements.AbstractInfusion;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodDefinition;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodImplementation;

public class InternalInfusion extends AbstractInfusion
{
	
	public InternalInfusion(InternalHeader header)
	{
		super();
		add(this.header = header);
		add(this.classList = new InternalClassList());
		add(this.methodDefinitionList = new InternalMethodDefinitionList());
		add(this.methodImplementationList = new InternalMethodImplementationList());
		add(this.staticFieldList = new InternalStaticFieldList());
		add(this.stringTable = new InternalStringTable(this));
		add(this.referencedInfusionList = new InternalReferencedInfusionList());
	}
	
	private AbstractMethodDefinition insertMethodDefinition(AbstractMethodDefinition methodDefinition)
	{
		// check if an equal method definition already exists in either the local
		// method definition list or in one of the imported infusions
		AbstractMethodDefinition ret = lookupMethodDefinition(methodDefinition.getName(), methodDefinition.getSignature());
		
		// if no alternative found, insert the definition into the definition list
		if (ret==null)
			ret = methodDefinitionList.add(methodDefinition);
		
		return ret;		
	}

	public void addJavaClass(JavaClass javaClass)
	{
		// add class to the class list
		InternalClassDefinition classDef = InternalClassDefinition.fromJavaClass(javaClass, this);
		classList.add(classDef);

		// extract methods
		for (org.apache.bcel.classfile.Method classMethod : javaClass.getMethods())
		{
			// create a method definition for this method
			AbstractMethodDefinition methodDef = 
				insertMethodDefinition(InternalMethodDefinition.fromMethod(classMethod));

			// if the class is not an interface, add its method implementations
			if (!javaClass.isInterface())
			{
				AbstractMethodImplementation methodImpl = 
					InternalMethodImplementation.fromMethod(classDef, methodDef, classMethod, javaClass.getSourceFileName());
				methodImpl = methodImplementationList.add(methodImpl);

				// hook these two together to form a method, but only if
				// non-static and non-private
				if ((!classMethod.isStatic()) && (!classMethod.isPrivate()))
				{
					InternalMethod method = new InternalMethod(methodDef, methodImpl);
					classDef.add(method);
				}
			}

		}

		// extract static fields
		for (org.apache.bcel.classfile.Field classField : javaClass.getFields())
		{
			if (classField.isStatic())
			{
				AbstractField field = InternalField.fromField(classField, classDef, this);
				staticFieldList.add(field);
			}
		}

	}

	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}
}
