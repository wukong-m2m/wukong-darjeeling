package org.csiro.darjeeling.infuser.structure.lightweightmethods;

import java.util.ArrayList;
import org.csiro.darjeeling.infuser.structure.BaseType;
import org.csiro.darjeeling.infuser.bytecode.*;
import org.csiro.darjeeling.infuser.bytecode.instructions.*;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodImplementation;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodDefinition;

public class LightweightMethod {
	public String className;
	public String methodName;
	private BaseType returnType;
	private BaseType[] parameters;
	private int maxIntStack;
	private int maxRefStack;

	public ArrayList<InstructionHandle> getInstructionHandles() { return null; }

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

	private LightweightMethod(String className, String methodName, BaseType returnType, BaseType[] parameters) {
		this.className = className;
		this.methodName = methodName;
		// It would be nice if we could determine this from the CodeBlock, but we need the information
		// when we process the INVOKELIGHT instruction, and it's not guaranteed the Lightweight method
		// will already have been processed at that time.
		// Would be good to change this sometime 
		this.returnType = returnType;
		this.parameters = parameters;
		this.maxIntStack = 1000000; // Will be set later in determineMaxStackDepth
		this.maxRefStack = 1000000; // Will be set later in determineMaxStackDepth
	}

	private static InstructionHandle addInstruction(ArrayList<InstructionHandle> instructionHandles, Instruction instruction) {
		InstructionHandle i = new InstructionHandle(instruction);
		instructionHandles.add(i);
		return i;
	}
	private static InstructionHandle addInstructionHandle(ArrayList<InstructionHandle> instructionHandles, InstructionHandle i) {
		instructionHandles.add(i);
		return i;		
	}

	public int getParameterIntStack() {
		int slots = 0;
		for (BaseType p : this.parameters) {
			slots += p.getNrIntegerSlots();
		}
		return slots;
	}

	public int getParameterRefStack() {
		int slots = 0;
		for (BaseType p : this.parameters) {
			slots += p.getNrReferenceSlots();
		}
		return slots;
	}

	public int getMaxIntStack() {
		return maxIntStack;
	}

	public int getMaxRefStack() {
		return maxRefStack;
	}

	private class DummyMethodImplementation extends InternalMethodImplementation {
		private class DummyMethodDefinition extends AbstractMethodDefinition {
			private BaseType returnType;
			private BaseType[] argumentTypes;

			public DummyMethodDefinition(BaseType returnType, BaseType[] argumentTypes) {
				super("","");
				this.returnType = returnType;
				this.argumentTypes = argumentTypes;
			}

			@Override
			public BaseType[] getArgumentTypes() {
				return argumentTypes;
			}

			@Override
			public BaseType getReturnType() {
				return returnType;
			}
		}

		public DummyMethodImplementation(BaseType returnType, BaseType[] argumentTypes) {
			this.methodDefinition = new DummyMethodDefinition(returnType, argumentTypes);
		}
	}

	public void determineMaxStackDepth() {
		InternalMethodImplementation dummyMethodImpl = new DummyMethodImplementation(this.returnType, this.parameters);
		CodeBlock dummy = CodeBlock.fromLightweightMethod(this, dummyMethodImpl);
		maxIntStack = dummy.getMaxStack() - dummy.getMaxRefStack();
		maxRefStack = dummy.getMaxRefStack() - 1; // -1 because CalculateMaxStack does a +1 for unknown reasons. We should remove this -1 if that ever changes.
		System.err.println("Stack depth for light method " + methodName + " int: " + maxIntStack + " ref: " + maxRefStack);
	}

	public static void registerLightweightMethod(LightweightMethod l) {
		l.determineMaxStackDepth();
		lightweightMethods.add(l);
	}

	static {
		lightweightMethods = new ArrayList<LightweightMethod>();

        registerLightweightMethod(coremark_ee_isdigit_lightweight());
        registerLightweightMethod(testISWAP());
		registerLightweightMethod(isOddShort());
		registerLightweightMethod(isOddInt());
		registerLightweightMethod(isNull());
		registerLightweightMethod(timesTenTestHighStackShort());
		registerLightweightMethod(timesTenTestHighStackRef());
	}

	private static LightweightMethod coremark_ee_isdigit_lightweight() {
		return new LightweightMethod("javax.rtcbench.CoreState", "ee_isdigit_lightweight", BaseType.Short,  new BaseType[] { BaseType.Short }) {
			@Override
			public ArrayList<InstructionHandle> getInstructionHandles() {
				ArrayList<InstructionHandle> l = new ArrayList<InstructionHandle>();
				InstructionHandle brtarget0 = new InstructionHandle(new BranchTargetInstruction(Opcode.BRTARGET));
				InstructionHandle brtarget1 = new InstructionHandle(new BranchTargetInstruction(Opcode.BRTARGET));

				// idup
				// bspush 48
				// if_scmplt brtarget 0
				// bspush 57
				// if_scmpgt brtarget 1
				// sconst_1
				// sreturn
				// brtarget 0
				// ipop // To clear the copy of the parameter. We won't need anymore.
				// brtarget 1
				// sconst_0
				// sreturn

				addInstruction(l, new StackInstruction(Opcode.IDUP));
				addInstruction(l, new ImmediateBytePushInstruction(Opcode.BSPUSH, 48));
				addInstruction(l, new BranchInstruction(Opcode.IF_SCMPLT, 0)).setBranchHandle(brtarget0);
				addInstruction(l, new ImmediateBytePushInstruction(Opcode.BSPUSH, 57));
				addInstruction(l, new BranchInstruction(Opcode.IF_SCMPGT, 0)).setBranchHandle(brtarget1);
				addInstruction(l, new ConstantPushInstruction(Opcode.SCONST_1, 1));
				addInstruction(l, new SimpleInstruction(Opcode.SRETURN));
				addInstructionHandle(l, brtarget0);
				addInstruction(l, new StackInstruction(Opcode.IPOP));				
				addInstructionHandle(l, brtarget1);
				addInstruction(l, new ConstantPushInstruction(Opcode.SCONST_0, 0));
				addInstruction(l, new SimpleInstruction(Opcode.SRETURN));

				return l;
			}
		};
	}

	private static LightweightMethod testISWAP() {
		return new LightweightMethod("javax.rtcbench.RTCBenchmark", "testISWAP", BaseType.Int, new BaseType[] { BaseType.Short, BaseType.Int }) {
			@Override
			public ArrayList<InstructionHandle> getInstructionHandles() {
				ArrayList<InstructionHandle> l = new ArrayList<InstructionHandle>();

				// iswap_x
				// s2i
				// iadd
				// ireturn

                addInstruction(l, new StackInstruction(Opcode.ISWAP_X));
				addInstruction(l, new ArithmeticInstruction(Opcode.S2I));
				addInstruction(l, new ArithmeticInstruction(Opcode.IADD));
				addInstruction(l, new SimpleInstruction(Opcode.IRETURN));

				return l;
			}
		};
	}
	private static LightweightMethod isOddShort() {
		return new LightweightMethod("javax.rtcbench.RTCBenchmark", "isOddShort",  BaseType.Short, new BaseType[] { BaseType.Short }) {
			@Override
			public ArrayList<InstructionHandle> getInstructionHandles() {
				ArrayList<InstructionHandle> l = new ArrayList<InstructionHandle>();

				InstructionHandle brtarget0 = new InstructionHandle(new BranchTargetInstruction(Opcode.BRTARGET));

				// sconst_1
				// sand
				// sreturn

				addInstruction(l, new ConstantPushInstruction(Opcode.SCONST_1, 1));
				addInstruction(l, new ArithmeticInstruction(Opcode.SAND));
				addInstruction(l, new SimpleInstruction(Opcode.SRETURN));

				return l;
			}
		};
	}

	private static LightweightMethod isOddInt() {
		return new LightweightMethod("javax.rtcbench.RTCBenchmark", "isOddInt",  BaseType.Short, new BaseType[] { BaseType.Int }) {
			@Override
			public ArrayList<InstructionHandle> getInstructionHandles() {
				ArrayList<InstructionHandle> l = new ArrayList<InstructionHandle>();

				// i2s
				// sconst_1
				// sand
				// sreturn

				addInstruction(l, new ExplicitCastInstruction(Opcode.I2S));
				addInstruction(l, new ConstantPushInstruction(Opcode.SCONST_1, 1));
				addInstruction(l, new ArithmeticInstruction(Opcode.SAND));
				addInstruction(l, new SimpleInstruction(Opcode.SRETURN));

				return l;
			}
		};
	}

	private static LightweightMethod isNull() {
		return new LightweightMethod("javax.rtcbench.RTCBenchmark", "isNull",  BaseType.Short, new BaseType[] { BaseType.Ref }) {
			@Override
			public ArrayList<InstructionHandle> getInstructionHandles() {
				ArrayList<InstructionHandle> l = new ArrayList<InstructionHandle>();
				InstructionHandle brtarget0 = new InstructionHandle(new BranchTargetInstruction(Opcode.BRTARGET));

				// ifnull brtarget 0
				// sconst_1
				// sret
				// brtarget 0
				// sconst_0
				// sret

				addInstruction(l, new BranchInstruction(Opcode.IFNULL, 0)).setBranchHandle(brtarget0);
				addInstruction(l, new ConstantPushInstruction(Opcode.SCONST_0, 0));
				addInstruction(l, new SimpleInstruction(Opcode.SRETURN));
				addInstructionHandle(l, brtarget0);
				addInstruction(l, new ConstantPushInstruction(Opcode.SCONST_1, 1));
				addInstruction(l, new SimpleInstruction(Opcode.SRETURN));

				return l;
			}
		};
	}

	private static LightweightMethod timesTenTestHighStackShort() {
		return new LightweightMethod("javax.rtcbench.RTCBenchmark", "timesTenTestHighStackShort",  BaseType.Short, new BaseType[] { BaseType.Short }) {
			@Override
			public ArrayList<InstructionHandle> getInstructionHandles() {
				ArrayList<InstructionHandle> l = new ArrayList<InstructionHandle>();

				// test if the infuser handles the case where the lightweight method has a stack higher than the parameters,
				// and higher than the calling method correctly. it needs to increase maxIntStack for the caller.

				// idup
				// idup
				// idup
				// idup
				// idup
				// idup
				// idup
				// idup
				// idup
				// sadd
				// sadd
				// sadd
				// sadd
				// sadd
				// sadd
				// sadd
				// sadd
				// sadd
				// bspush 42
				// sreturn

				addInstruction(l, new StackInstruction(Opcode.IDUP));
				addInstruction(l, new StackInstruction(Opcode.IDUP));
				addInstruction(l, new StackInstruction(Opcode.IDUP));
				addInstruction(l, new StackInstruction(Opcode.IDUP));
				addInstruction(l, new StackInstruction(Opcode.IDUP));
				addInstruction(l, new StackInstruction(Opcode.IDUP));
				addInstruction(l, new StackInstruction(Opcode.IDUP));
				addInstruction(l, new StackInstruction(Opcode.IDUP));
				addInstruction(l, new StackInstruction(Opcode.IDUP));
				addInstruction(l, new ArithmeticInstruction(Opcode.SADD));
				addInstruction(l, new ArithmeticInstruction(Opcode.SADD));
				addInstruction(l, new ArithmeticInstruction(Opcode.SADD));
				addInstruction(l, new ArithmeticInstruction(Opcode.SADD));
				addInstruction(l, new ArithmeticInstruction(Opcode.SADD));
				addInstruction(l, new ArithmeticInstruction(Opcode.SADD));
				addInstruction(l, new ArithmeticInstruction(Opcode.SADD));
				addInstruction(l, new ArithmeticInstruction(Opcode.SADD));
				addInstruction(l, new ArithmeticInstruction(Opcode.SADD));
				addInstruction(l, new SimpleInstruction(Opcode.SRETURN));

				return l;
			}
		};
	}

	private static LightweightMethod timesTenTestHighStackRef() {
		return new LightweightMethod("javax.rtcbench.RTCBenchmark", "timesTenTestHighStackRef", BaseType.Short, new BaseType[] { BaseType.Ref }) {
			@Override
			public ArrayList<InstructionHandle> getInstructionHandles() {
				ArrayList<InstructionHandle> l = new ArrayList<InstructionHandle>();

				// test if the infuser handles the case where the lightweight method has a stack higher than the parameters,
				// and higher than the calling method correctly. it needs to increase maxIntStack for the caller.

				// adup
				// adup
				// adup
				// adup
				// adup
				// adup
				// adup
				// adup
				// adup
				// apop
				// apop
				// apop
				// apop
				// apop
				// apop
				// apop
				// apop
				// apop
				// apop
				// sreturn

				addInstruction(l, new StackInstruction(Opcode.ADUP));
				addInstruction(l, new StackInstruction(Opcode.ADUP));
				addInstruction(l, new StackInstruction(Opcode.ADUP));
				addInstruction(l, new StackInstruction(Opcode.ADUP));
				addInstruction(l, new StackInstruction(Opcode.ADUP));
				addInstruction(l, new StackInstruction(Opcode.ADUP));
				addInstruction(l, new StackInstruction(Opcode.ADUP));
				addInstruction(l, new StackInstruction(Opcode.ADUP));
				addInstruction(l, new StackInstruction(Opcode.ADUP));
				addInstruction(l, new ArithmeticInstruction(Opcode.APOP));
				addInstruction(l, new ArithmeticInstruction(Opcode.APOP));
				addInstruction(l, new ArithmeticInstruction(Opcode.APOP));
				addInstruction(l, new ArithmeticInstruction(Opcode.APOP));
				addInstruction(l, new ArithmeticInstruction(Opcode.APOP));
				addInstruction(l, new ArithmeticInstruction(Opcode.APOP));
				addInstruction(l, new ArithmeticInstruction(Opcode.APOP));
				addInstruction(l, new ArithmeticInstruction(Opcode.APOP));
				addInstruction(l, new ArithmeticInstruction(Opcode.APOP));
				addInstruction(l, new ArithmeticInstruction(Opcode.APOP));
				addInstruction(l, new ImmediateBytePushInstruction(Opcode.BSPUSH, 42));
				addInstruction(l, new SimpleInstruction(Opcode.SRETURN));

				return l;
			}
		};
	}
}
