package org.csiro.darjeeling.infuser.structure.lightweightmethods;

import java.util.ArrayList;
import org.csiro.darjeeling.infuser.structure.BaseType;
import org.csiro.darjeeling.infuser.bytecode.*;
import org.csiro.darjeeling.infuser.bytecode.instructions.*;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodImplementation;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodDefinition;

// KNOWN ISSUES
// - We currently don't support calling lightweight methods in another infusion. This just because we haven't implemented it yet since it's not necessary for our benchmarks, but there's no reason it couldn't be done.

public class LightweightMethod {
	public String className;
	public String methodName;
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

	// A common mistake is to put @Lightweight methods after the place where they're called in nested methods.
	// The result is that the original INVOKESTATIC is not changed to INVOKELIGHT, which causes a crash at runtime.
	// This list contains all methods for which isLightweightMethod has returned false before. If we're about to
	// return true, this is a bug and we should stop.
	private static ArrayList<String> checkedForLightweight = new ArrayList<String>();
	public static boolean isLightweightMethod(String className, String methodName) {
		String fullname = className + "." + methodName;
		LightweightMethod lightweightMethod = getLightweightMethod(className, methodName);

		if (lightweightMethod == null) {
			checkedForLightweight.add(fullname);
			return false;
		} else {
			return true;
		}
	}

	public static LightweightMethod getLightweightMethod(String className, String methodName) {
		for (LightweightMethod lightweightMethod : lightweightMethods) {
			if (lightweightMethod.className.equals(className) && lightweightMethod.methodName.equals(methodName))
				return lightweightMethod;
		}
		return null;		
	}

	protected LightweightMethod(String className, String methodName) {
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
			System.err.println("No methodImpl set when calling getMaxIntStack in LightweightMethod.java (" + this.className + "." + this.methodName + "). Did you forget -Pno-proguard?");
		}
		return this.methodImpl.getMaxStack() - this.methodImpl.getMaxRefStack();
	}

	public int getMaxRefStack() {
		if (this.methodImpl == null) {
			System.err.println("No methodImpl set when calling getMaxRefStack in LightweightMethod.java (" + this.className + "." + this.methodName + "). Did you forget -Pno-proguard?");
		}
		return this.methodImpl.getMaxRefStack();
	}

	public int getTotalLocalVariableSlots() {
		if (this.methodImpl == null) {
			System.err.println("No methodImpl set when calling getLocalVariableCount in LightweightMethod.java (" + this.className + "." + this.methodName + "). Did you forget -Pno-proguard?");
		}
		return this.methodImpl.getIntegerLocalVariableCount()
				+ this.methodImpl.getReferenceLocalVariableCount()
				+ this.methodImpl.getMaxLightweightMethodLocalVariableCount()
				+ (this.methodImpl.usesSIMUL_INVOKELIGHT_MARKLOOP() ? 1 : 0); // If the lightweight method uses SIMUL or INVOKELIGHT, we need to reserve an extra slot since we can't store the return address in R18:R19. Instead we have to store it in the stack frame at a cost of 8 extra cycles.
	}

	public static void guardNeverReturnedFalseFromIsLightweightMethod(String className, String methodName) {
		String fullname = className + "." + methodName;
		for (String previouslyReturnedFalseForMethod : checkedForLightweight) {
			if (fullname.equals(previouslyReturnedFalseForMethod)) {
				throw new Error("About to register " + fullname + " as lightweight, while isLightweightMethod previously returned false. This means the previous INVOKESTATIC wasn't changed to INVOKELIGHT, which is a bug. Probably the method is defined after the point where it's called.");
			}
		}
	}

	public static LightweightMethod registerJavaLightweightMethod(String className, String methodName) {
		guardNeverReturnedFalseFromIsLightweightMethod(className, methodName);
		LightweightMethod l = new LightweightMethod(className, methodName);
		lightweightMethods.add(l);
		return l;
	}

	public static void registerHardcodedLightweightMethod(LightweightMethod l) {
		guardNeverReturnedFalseFromIsLightweightMethod(l.className, l.methodName);
		lightweightMethods.add(l);
	}

	public void setMethodImpl(InternalMethodImplementation methodImpl) {
		this.methodImpl = methodImpl;
		// System.err.println("Stack depth and locals for light method " + methodImpl.toString() + " int: " + this.getMaxIntStack() + " ref: " + this.getMaxRefStack() + " locals: " + this.getLocalVariableCount());
	}

	static {
		lightweightMethods = new ArrayList<LightweightMethod>();

        registerHardcodedLightweightMethod(LightweightMethodImplementations.fft_FIX_MPY_lightweight());
        registerHardcodedLightweightMethod(LightweightMethodImplementations.coremark_ee_isdigit_lightweight());
        registerHardcodedLightweightMethod(LightweightMethodImplementations.testISWAP());
        registerHardcodedLightweightMethod(LightweightMethodImplementations.testLOAD_STORE());
		registerHardcodedLightweightMethod(LightweightMethodImplementations.isOddShort());
		registerHardcodedLightweightMethod(LightweightMethodImplementations.isOddInt());
		registerHardcodedLightweightMethod(LightweightMethodImplementations.isNull());
		registerHardcodedLightweightMethod(LightweightMethodImplementations.timesTenTestHighStackShort());
		registerHardcodedLightweightMethod(LightweightMethodImplementations.timesTenTestHighStackRef());
	}
}
