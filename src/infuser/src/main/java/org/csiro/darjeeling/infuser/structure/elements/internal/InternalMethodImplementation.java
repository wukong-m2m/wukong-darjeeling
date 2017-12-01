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

import org.apache.bcel.classfile.AnnotationEntry;
import org.apache.bcel.classfile.ElementValuePair;
import org.apache.bcel.classfile.Code;
import org.apache.bcel.classfile.Method;
import org.apache.bcel.generic.ConstantPoolGen;
import org.apache.bcel.generic.Type;
import org.csiro.darjeeling.infuser.Infuser;
import org.csiro.darjeeling.infuser.bytecode.CodeBlock;
import org.csiro.darjeeling.infuser.structure.BaseType;
import org.csiro.darjeeling.infuser.structure.ElementVisitor;
import org.csiro.darjeeling.infuser.structure.GlobalId;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodDefinition;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodImplementation;
import org.csiro.darjeeling.infuser.structure.lightweightmethods.LightweightMethod;

public class InternalMethodImplementation extends AbstractMethodImplementation
{
	
	protected Code code;
	private String fileName;
	protected CodeBlock codeBlock;
	
	private boolean isSynchronized;
	private boolean isNative;
	private boolean isLightweight;

	protected InternalMethodImplementation()
	{
	}
	
	public static int javaMethodGetLightweightRank(Method method) {
		for (AnnotationEntry entry : method.getAnnotationEntries()) {
			if (entry.getAnnotationType().contains("Lightweight")) {
				for (ElementValuePair pair : entry.getElementValuePairs()) {
					if (pair.getNameString().equals("rank")) {
						return Integer.parseInt(pair.getValue().toString());
					}
				}
				// If no rank is found return 0
				return 0;
			}
		}
		// If not a lightweight method, sort them at the end (in InternalInfusion.java)
		return Integer.MAX_VALUE;
	}

	public static boolean javaMethodIsLightweight(Method method) {
        if (!Infuser.getUseLightweightMethods()) {
            // Dan niet.
            return false;
        }

		for (AnnotationEntry entry : method.getAnnotationEntries()) {
			if (entry.getAnnotationType().contains("Lightweight")) {
				return true;
			}
		}
		return false;
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

		methodImpl.isLightweight = javaMethodIsLightweight(method);
		methodImpl.isNative = method.isNative() && !methodImpl.isLightweight; 

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
		// We now have 3 types of methods:
		// - Normal Java methods
		// - Hardcoded Lightweight methods implemented in LightweightMethodImplementation.java
		//   These are marked native in the java code that uses them and will be replaced by the infuser with the hardcoded JVM implementation (may be stack-only)
		// - Java methods marked lightweight
		//   These are implemented as normal Java methods but will be called as a lightweight method. This means the parameters will be on the stack when the method
		//   is called and the infuser should generate the necessary dummy parameter instructions and STOREs to initialise the corresponding local variables.
		//   Note that these methods may not trigger the garbage collector!


		if (isLightweight) {
			System.err.println("Adding lightweight method " + this.parentClass.getName() + "." + this.methodDefinition.getName());
			LightweightMethod lightweightMethod;
			if (code!=null) {
				// Java lightweight method
				lightweightMethod = LightweightMethod.registerJavaLightweightMethod(this.parentClass.getName(), this.methodDefinition.getName());
				codeBlock = CodeBlock.fromCode(code, this, infusion, new ConstantPoolGen(code.getConstantPool()));
			} else {
				// Hardcoded JVM lightweight method: this already has the hardcoded LightweightMethod object registered.
				lightweightMethod = LightweightMethod.getLightweightMethod(this.parentClass.getName(), this.methodDefinition.getName());
				codeBlock = CodeBlock.fromLightweightMethod(lightweightMethod, this);
			}

			// If it is a lightweight method, regardless hardcoded or Java, set the implementation, so we can find it when processing INVOKELIGHT
			lightweightMethod.setMethodImpl(this);
		} else {
			// Normal method
			if (code!=null) {
				codeBlock = CodeBlock.fromCode(code, this, infusion, new ConstantPoolGen(code.getConstantPool()));
			}
		}

		// System.err.println("Processed " + this 
		// 	             + " int slots: " + this.getIntegerLocalVariableCount()
		// 	             + " ref slots: " +  + this.getReferenceLocalVariableCount()
		// 	             + " reserved lw slots: " + this.getMaxLightweightMethodLocalVariableCount()
		// 	             + (this.isLightweight() ? " (lightweight)" : "")
		// 	             + (this.usesSIMUL_INVOKELIGHT_MARKLOOP() ? " (uses simul or invokelight)" : ""));
	}
	
	public CodeBlock getCodeBlock()
	{
		return codeBlock;
	}
	
	public Code getCode()
	{
		return code;
	}
	
	public int getMaxTotalStack()
	{
		if (codeBlock==null) return 0; else
			return codeBlock.getMaxTotalStack();
	}

	public int getMaxIntStack()
	{
		if (codeBlock==null) return 0; else
			return codeBlock.getMaxIntStack();
	}

	public int getMaxRefStack()
	{
		if (codeBlock==null) return 0; else
			return codeBlock.getMaxRefStack();
	}

	public boolean isNative()
	{
		return isNative;
	}

	public boolean isLightweight()
	{
		return isLightweight;
	}

	public boolean usesSIMUL_INVOKELIGHT_MARKLOOP()
	{
		if (codeBlock==null) return false; else
			return codeBlock.usesSIMUL_INVOKELIGHT_MARKLOOP();
	}

	public boolean usesStaticFields()
	{
		if (codeBlock==null) return false; else
			return codeBlock.usesStaticFields();
	}

	public int getNumberOfBranchTargets()
	{
		if (codeBlock==null) return 0; else
			return codeBlock.getNumberOfBranchTargets();
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

	public int getMaxLightweightMethodLocalVariableCount()
	{
		if (codeBlock==null) return 0; else
			return codeBlock.getMaxLightweightMethodLocalVariableCount();
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
