package org.csiro.darjeeling.infuser.structure.lightweightmethods;

import java.util.ArrayList;
import org.csiro.darjeeling.infuser.structure.BaseType;
import org.csiro.darjeeling.infuser.bytecode.*;
import org.csiro.darjeeling.infuser.bytecode.instructions.*;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodImplementation;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodDefinition;

// KNOWN ISSUES
// - We currently don't support calling lightweight methods in another infusion. This just because we haven't implemented it yet since it's not necessary for our benchmarks, but there's no reason it couldn't be done.
// - The lightweight methods need to come first in the infusion. We should modify the processing of methods to make sure it's done in the right order, but for our benchmarks we just do it by reorganising them in the class file.
//   Again this is just to save some implementation time since it works well enough to do our experiments for now. 
//   This causes problems when a method in a superclass invokes a lightweight method in a subclass, since the superclass' method will come first in the infusion.

public class LightweightMethod {
	public String className;
	public String methodName;
	private boolean isJavaMethod;
	private ArrayList<LocalVariable> localVariables;
	private ArrayList<InstructionHandle> instructionHandles;
	private InternalMethodImplementation methodImpl = null;

	public void setupLightweightMethod() {}

	public ArrayList<LocalVariable> getLocalVariables() {
		return this.localVariables;
	}
	public ArrayList<InstructionHandle> getInstructionHandles() {
		return this.instructionHandles;
	}

	private static ArrayList<LightweightMethod> lightweightMethods;

	public static boolean isLightweightMethod(String className, String methodName) {
		return getLightweightMethod(className, methodName) != null;
	}

	public static LightweightMethod getLightweightMethod(String className, String methodName) {
		for (LightweightMethod lightweightMethod : lightweightMethods) {
			if (lightweightMethod.className.equals(className) && lightweightMethod.methodName.equals(methodName))
				return lightweightMethod;
		}
		return null;		
	}

	protected LightweightMethod(String className, String methodName, boolean isJavaMethod) {
		this.isJavaMethod = isJavaMethod;
		this.className = className;
		this.methodName = methodName;
		// It would be nice if we could determine this from the CodeBlock, but we need the information
		// when we process the INVOKELIGHT instruction, and it's not guaranteed the Lightweight method
		// will already have been processed at that time.
		// Would be good to change this sometime 
		localVariables = new ArrayList<LocalVariable>();
		instructionHandles = new ArrayList<InstructionHandle>();
		this.methodImpl = null; // Will be set when we either generate the lightweight method from the definitions below, or from a normal method marked lightweight.
		this.setupLightweightMethod();
	}

	protected LocalVariable addLocalVariable(int slot, BaseType type) {
		LocalVariable localVariable = new LocalVariable(slot);
		if (type == BaseType.Ref) {
			localVariable.setReferenceIndex(slot);
		} else {
			localVariable.setIntegerIndex(slot);		
		}
		localVariable.setType(type);
		this.localVariables.add(localVariable);
		return localVariable;
	}
	protected InstructionHandle addInstruction(Instruction instruction) {
		InstructionHandle i = new InstructionHandle(instruction);
		this.instructionHandles.add(i);
		return i;
	}
	protected InstructionHandle addInstructionHandle(InstructionHandle i) {
		this.instructionHandles.add(i);
		return i;		
	}

	public int getParameterIntStack() {
		return this.methodImpl.getIntegerArgumentCount();
	}

	public int getParameterRefStack() {
		return this.methodImpl.getReferenceArgumentCount();
	}

	public int getMaxIntStack() {
		if (this.methodImpl == null) {
			System.err.println("No methodImpl set when calling getMaxIntStack in LightweightMethod.java. Did you forget -Pno-proguard or are the lightweight methods defined after the calling method in the Java class?");
		}
		return this.methodImpl.getMaxStack() - this.methodImpl.getMaxRefStack();
	}

	public int getMaxRefStack() {
		if (this.methodImpl == null) {
			System.err.println("No methodImpl set when calling getMaxRefStack in LightweightMethod.java. Did you forget -Pno-proguard or are the lightweight methods defined after the calling method in the Java class?");
		}
		return this.methodImpl.getMaxRefStack();
	}

	public int getLocalVariableCount() {
		if (this.methodImpl == null) {
			System.err.println("No methodImpl set when calling getLocalVariableCount in LightweightMethod.java. Did you forget -Pno-proguard or are the lightweight methods defined after the calling method in the Java class?");
		}
		return this.methodImpl.getIntegerLocalVariableCount() + this.methodImpl.getReferenceLocalVariableCount();
	}

	public static void registerLightweightMethod(LightweightMethod l) {
		lightweightMethods.add(l);
	}

	public void setMethodImpl(InternalMethodImplementation methodImpl) {
		this.methodImpl = methodImpl;
		System.err.println("Stack depth and locals for light method " + methodImpl.toString() + " int: " + this.getMaxIntStack() + " ref: " + this.getMaxRefStack() + " locals: " + this.getLocalVariableCount());
	}

	public boolean isJavaMethod() {
		return this.isJavaMethod;
	}

	static {
		lightweightMethods = new ArrayList<LightweightMethod>();

        registerLightweightMethod(LightweightMethodImplementations.fft_FIX_MPY_lightweight());
        registerLightweightMethod(LightweightMethodImplementations.coremark_ee_isdigit_lightweight());
        registerLightweightMethod(LightweightMethodImplementations.testISWAP());
        registerLightweightMethod(LightweightMethodImplementations.testLOAD_STORE());
		registerLightweightMethod(LightweightMethodImplementations.isOddShort());
		registerLightweightMethod(LightweightMethodImplementations.isOddInt());
		registerLightweightMethod(LightweightMethodImplementations.isNull());
		registerLightweightMethod(LightweightMethodImplementations.timesTenTestHighStackShort());
		registerLightweightMethod(LightweightMethodImplementations.timesTenTestHighStackRef());
	}
}
