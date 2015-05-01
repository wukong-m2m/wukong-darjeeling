/*
 * InternalMethodImplementation.java
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

import org.apache.bcel.classfile.Code;
import org.apache.bcel.classfile.Method;
import org.apache.bcel.generic.ConstantPoolGen;
import org.apache.bcel.generic.Type;
import org.csiro.darjeeling.infuser.bytecode.CodeBlock;
import org.csiro.darjeeling.infuser.structure.BaseType;
import org.csiro.darjeeling.infuser.structure.ElementVisitor;
import org.csiro.darjeeling.infuser.structure.GlobalId;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodDefinition;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodImplementation;

public class InternalMethodImplementation extends AbstractMethodImplementation
{
	
	protected Code code;
	private String fileName;
	protected CodeBlock codeBlock;
	
	private boolean isSynchronized;
	private boolean isNative;

	protected InternalMethodImplementation()
	{
	}
	
	public static InternalMethodImplementation fromMethod(InternalClassDefinition parentClass, AbstractMethodDefinition methodDef, Method method, String fileName)
	{
		InternalMethodImplementation methodImpl = new InternalMethodImplementation();
		
		methodImpl.fileName = fileName;
		
		methodImpl.parentClass = parentClass;
		methodImpl.methodDefinition = methodDef;
		methodImpl.code = method.getCode();
		methodImpl.setStatic(method.isStatic());		
		
		methodImpl.integerArgumentCount = 0;
		methodImpl.referenceArgumentCount = 0;
		
		methodImpl.isSynchronized = method.isSynchronized();		
		methodImpl.isNative = method.isNative();		

		for (Type type : method.getArgumentTypes())
		{
			BaseType bType = BaseType.fromBCELType(type);
			methodImpl.referenceArgumentCount += bType.getNrReferenceSlots();
			methodImpl.integerArgumentCount += bType.getNrIntegerSlots();
		}
		
		return methodImpl;
	}
	
	public void processCode(InternalInfusion infusion)
	{
		if (code==null) return;
		codeBlock = CodeBlock.fromCode(code, this, infusion, new ConstantPoolGen(code.getConstantPool()));
	}
	
	public CodeBlock getCodeBlock()
	{
		return codeBlock;
	}
	
	public Code getCode()
	{
		return code;
	}
	
	public int getMaxStack()
	{
		if (codeBlock==null) return 0; else
			return codeBlock.getMaxStack();
	}

	public boolean isNative()
	{
		return isNative;
	}
	
	public int getReferenceLocalVariableCount()
	{
		if (codeBlock==null) return 0; else
			return codeBlock.getReferenceLocalVariableCount();
	}
	
	public int getIntegerLocalVariableCount()
	{
		if (codeBlock==null) return 0; else
			return codeBlock.getIntegerLocalVariableCount();
	}

	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}

	@Override
	public GlobalId getMethodDefGlobalId()
	{
		return methodDefinition.getGlobalId();
	}

	@Override
	public GlobalId getParentClassGlobalId()
	{
		return parentClass.getGlobalId();
	}
	
	public String getFileName()
	{
		return fileName;
	}
	
	public boolean isSynchronized()
	{
		return isSynchronized;
	}
	
}
