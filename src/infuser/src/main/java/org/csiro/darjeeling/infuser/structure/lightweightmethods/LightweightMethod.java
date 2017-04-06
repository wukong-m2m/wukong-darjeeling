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

	private LightweightMethod(String className, String methodName, BaseType[] parameters) {
		this.className = className;
		this.methodName = methodName;
		// It would be nice if we could determine this from the CodeBlock, but we need the information
		// when we process the INVOKELIGHT instruction, and it's not guaranteed the Lightweight method
		// will already have been processed at that time.
		// Would be good to change this sometime 
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

	public int getMaxIntStack() {
		return maxIntStack;
	}

	public int getMaxRefStack() {
		return maxRefStack;
	}

	private class DummyMethodImplementation extends InternalMethodImplementation {
		private class DummyMethodDefinition extends AbstractMethodDefinition {
			private BaseType[] arguments;

			public DummyMethodDefinition(BaseType[] arguments) {
				super("","");
				this.arguments = arguments;
			}

			@Override
			public BaseType[] getArgumentTypes() {
				return arguments;
			}
		}

		public DummyMethodImplementation(BaseType[] arguments) {
			this.methodDefinition = new DummyMethodDefinition(arguments);
		}
	}

	public void determineMaxStackDepth() {
		InternalMethodImplementation dummyMethodImpl = new DummyMethodImplementation(this.parameters);
		CodeBlock dummy = CodeBlock.fromLightweightMethod(this, dummyMethodImpl);
		maxIntStack = dummy.getMaxStack()-1;    // -1 because CalculateMaxStack does a +1 for unknown reasons. We should remove this -1 if that ever changes.
		maxRefStack = dummy.getMaxRefStack()-1;
		System.err.println("Stack depth for light method " + methodName + " int: " + maxIntStack + " ref: " + maxRefStack);
	}

	public static void registerLightweightMethod(LightweightMethod l) {
		l.determineMaxStackDepth();
		lightweightMethods.add(l);
	}

	static {
		lightweightMethods = new ArrayList<LightweightMethod>();

		// registerLightweightMethod(ee_isdigit_light());
		registerLightweightMethod(isOddShort());
		registerLightweightMethod(isOddInt());
		// registerLightweightMethod(isNull());
		// registerLightweightMethod(addXZand1ifYnotnull());
		registerLightweightMethod(timesTenTestHighStackShort());
		// registerLightweightMethod(timesTenTestHighStackInt());
	}

	private static LightweightMethod ee_isdigit_light() {
		return new LightweightMethod("javax.rtcbench.CoreState", "ee_isdigit_light", new BaseType[] { BaseType.Short }) {
			@Override
			public ArrayList<InstructionHandle> getInstructionHandles() {
				ArrayList<InstructionHandle> l = new ArrayList<InstructionHandle>();
				InstructionHandle brtarget0 = new InstructionHandle(new BranchTargetInstruction(Opcode.BRTARGET));

				// idup
				// bspush 48
				// if_scmplt brtarget 0
				// idup
				// bspush 57
				// if_scmpgt brtarget 0
				// sconst_1
				// sreturn
				// brtarget 0
				// sconst_0
				// sreturn

				addInstruction(l, new StackInstruction(Opcode.IDUP));
				addInstruction(l, new ConstantPushInstruction(Opcode.BSPUSH, 48));
				addInstruction(l, new BranchInstruction(Opcode.SIFLT, 0)).setBranchHandle(brtarget0);
				addInstruction(l, new StackInstruction(Opcode.IDUP));
				addInstruction(l, new ConstantPushInstruction(Opcode.BSPUSH, 57));
				addInstruction(l, new BranchInstruction(Opcode.SIFGT, 0)).setBranchHandle(brtarget0);
				addInstruction(l, new ConstantPushInstruction(Opcode.SCONST_1, 1));
				addInstruction(l, new SimpleInstruction(Opcode.SRETURN));
				addInstructionHandle(l, brtarget0);
				addInstruction(l, new ConstantPushInstruction(Opcode.SCONST_0, 0));
				addInstruction(l, new SimpleInstruction(Opcode.SRETURN));

				return l;
			}
		};
	}

	private static LightweightMethod isOddShort() {
		return new LightweightMethod("javax.rtcbench.RTCBenchmark", "isOddShort", new BaseType[] { BaseType.Short }) {
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
		return new LightweightMethod("javax.rtcbench.RTCBenchmark", "isOddInt", new BaseType[] { BaseType.Int }) {
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
		return new LightweightMethod("javax.rtcbench.RTCBenchmark", "isNull", new BaseType[] { BaseType.Ref }) {
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
				addInstruction(l, new ConstantPushInstruction(Opcode.SCONST_1, 1));
				addInstruction(l, new SimpleInstruction(Opcode.SRETURN));
				addInstructionHandle(l, brtarget0);
				addInstruction(l, new ConstantPushInstruction(Opcode.SCONST_0, 0));
				addInstruction(l, new SimpleInstruction(Opcode.SRETURN));

				return l;
			}
		};
	}

	private static LightweightMethod addXZand1ifYnotnull() {
		return new LightweightMethod("javax.rtcbench.RTCBenchmark", "addXZand1ifYnotnull", new BaseType[] { BaseType.Int, BaseType.Ref, BaseType.Short }) {
			@Override
			public ArrayList<InstructionHandle> getInstructionHandles() {
				ArrayList<InstructionHandle> l = new ArrayList<InstructionHandle>();
				InstructionHandle brtarget0 = new InstructionHandle(new BranchTargetInstruction(Opcode.BRTARGET));

				// stack: int, ref, short
				// note that we can't do (short, ref, int) since there's no way to s2i the short if it's not at the top of the stack

				// s2i
				// ifnull brtarget 0
				// iconst_1
				// iadd
				// brtarget 0
				// iadd
				// ireturn

				addInstruction(l, new ArithmeticInstruction(Opcode.IADD));
				addInstruction(l, new BranchInstruction(Opcode.IFNULL, 0)).setBranchHandle(brtarget0);
				addInstruction(l, new ConstantPushInstruction(Opcode.ICONST_1, 1));
				addInstruction(l, new ArithmeticInstruction(Opcode.IADD));
				addInstructionHandle(l, brtarget0);
				addInstruction(l, new ArithmeticInstruction(Opcode.IADD));
				addInstruction(l, new SimpleInstruction(Opcode.IRETURN));

				return l;
			}
		};
	}

	private static LightweightMethod timesTenTestHighStackShort() {
		return new LightweightMethod("javax.rtcbench.RTCBenchmark", "timesTenTestHighStackShort", new BaseType[] { BaseType.Short }) {
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

	private static LightweightMethod timesTenTestHighStackInt() {
		return new LightweightMethod("javax.rtcbench.RTCBenchmark", "timesTenTestHighStackInt", new BaseType[] { BaseType.Int }) {
			@Override
			public ArrayList<InstructionHandle> getInstructionHandles() {
				ArrayList<InstructionHandle> l = new ArrayList<InstructionHandle>();

				// test if the infuser handles the case where the lightweight method has a stack higher than the parameters,
				// and higher than the calling method correctly. it needs to increase maxIntStack for the caller.

				// idup2
				// idup2
				// idup2
				// idup2
				// idup2
				// idup2
				// idup2
				// idup2
				// idup2
				// iadd
				// iadd
				// iadd
				// iadd
				// iadd
				// iadd
				// iadd
				// iadd
				// iadd
				// ireturn

				addInstruction(l, new StackInstruction(Opcode.IDUP2));
				addInstruction(l, new StackInstruction(Opcode.IDUP2));
				addInstruction(l, new StackInstruction(Opcode.IDUP2));
				addInstruction(l, new StackInstruction(Opcode.IDUP2));
				addInstruction(l, new StackInstruction(Opcode.IDUP2));
				addInstruction(l, new StackInstruction(Opcode.IDUP2));
				addInstruction(l, new StackInstruction(Opcode.IDUP2));
				addInstruction(l, new StackInstruction(Opcode.IDUP2));
				addInstruction(l, new StackInstruction(Opcode.IDUP2));
				addInstruction(l, new ArithmeticInstruction(Opcode.IADD));
				addInstruction(l, new ArithmeticInstruction(Opcode.IADD));
				addInstruction(l, new ArithmeticInstruction(Opcode.IADD));
				addInstruction(l, new ArithmeticInstruction(Opcode.IADD));
				addInstruction(l, new ArithmeticInstruction(Opcode.IADD));
				addInstruction(l, new ArithmeticInstruction(Opcode.IADD));
				addInstruction(l, new ArithmeticInstruction(Opcode.IADD));
				addInstruction(l, new ArithmeticInstruction(Opcode.IADD));
				addInstruction(l, new ArithmeticInstruction(Opcode.IADD));
				addInstruction(l, new SimpleInstruction(Opcode.IRETURN));

				return l;
			}
		};
	}		
}
