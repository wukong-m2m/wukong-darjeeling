package org.csiro.darjeeling.infuser.structure.lightweightmethods;

import java.util.ArrayList;
import org.csiro.darjeeling.infuser.bytecode.*;
import org.csiro.darjeeling.infuser.bytecode.instructions.*;

public class LightweightMethod {
	public String className;
	public String methodName;

	private ArrayList<InstructionHandle> instructionHandles;

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

	private LightweightMethod(String className, String methodName) {
		this.className = className;
		this.methodName = methodName;
		this.instructionHandles = new ArrayList<InstructionHandle>();
	}

	private InstructionHandle addInstruction(Instruction instruction) {
		InstructionHandle i = new InstructionHandle(instruction);
		this.instructionHandles.add(i);
		return i;
	}
	private InstructionHandle addInstructionHandle(InstructionHandle i) {
		this.instructionHandles.add(i);
		return i;		
	}

	public ArrayList<InstructionHandle> getInstructionHandles() {
		return instructionHandles;
	}

	static {
		lightweightMethods = new ArrayList<LightweightMethod>();

		lightweightMethods.add(ee_isdigit_light());
		lightweightMethods.add(isOddShort());
		lightweightMethods.add(isOddInt());
		lightweightMethods.add(isNull());
		lightweightMethods.add(addXZand1ifYnotnull());
		lightweightMethods.add(timesTenTestHighStackShort());
		lightweightMethods.add(timesTenTestHighStackInt());
	}

	private static LightweightMethod ee_isdigit_light() {
		LightweightMethod l = new LightweightMethod("javax.rtcbench.CoreState", "ee_isdigit_light");
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

		l.addInstruction(new StackInstruction(Opcode.IDUP));
		l.addInstruction(new ConstantPushInstruction(Opcode.BSPUSH, 48));
		l.addInstruction(new BranchInstruction(Opcode.SIFLT, 0)).setBranchHandle(brtarget0);
		l.addInstruction(new StackInstruction(Opcode.IDUP));
		l.addInstruction(new ConstantPushInstruction(Opcode.BSPUSH, 57));
		l.addInstruction(new BranchInstruction(Opcode.SIFGT, 0)).setBranchHandle(brtarget0);
		l.addInstruction(new ConstantPushInstruction(Opcode.SCONST_1, 1));
		l.addInstruction(new SimpleInstruction(Opcode.SRETURN));
		l.addInstructionHandle(brtarget0);
		l.addInstruction(new ConstantPushInstruction(Opcode.SCONST_0, 0));
		l.addInstruction(new SimpleInstruction(Opcode.SRETURN));

		return l;
	}

	private static LightweightMethod isOddShort() {
		LightweightMethod l = new LightweightMethod("javax.rtcbench.RTCBenchmark", "isOddShort");
		InstructionHandle brtarget0 = new InstructionHandle(new BranchTargetInstruction(Opcode.BRTARGET));

		// sconst_1
		// sand
		// sreturn

		l.addInstruction(new ConstantPushInstruction(Opcode.SCONST_1, 1));
		l.addInstruction(new ArithmeticInstruction(Opcode.SAND));
		l.addInstruction(new SimpleInstruction(Opcode.SRETURN));

		return l;
	}

	private static LightweightMethod isOddInt() {
		LightweightMethod l = new LightweightMethod("javax.rtcbench.RTCBenchmark", "isOddInt");

		// i2s
		// sconst_1
		// sand
		// sreturn

		l.addInstruction(new ExplicitCastInstruction(Opcode.I2S));
		l.addInstruction(new ConstantPushInstruction(Opcode.SCONST_1, 1));
		l.addInstruction(new ArithmeticInstruction(Opcode.SAND));
		l.addInstruction(new SimpleInstruction(Opcode.SRETURN));

		return l;
	}

	private static LightweightMethod isNull() {
		LightweightMethod l = new LightweightMethod("javax.rtcbench.RTCBenchmark", "isNull");
		InstructionHandle brtarget0 = new InstructionHandle(new BranchTargetInstruction(Opcode.BRTARGET));

		// ifnull brtarget 0
		// sconst_1
		// sret
		// brtarget 0
		// sconst_0
		// sret

		l.addInstruction(new BranchInstruction(Opcode.IFNULL, 0)).setBranchHandle(brtarget0);
		l.addInstruction(new ConstantPushInstruction(Opcode.SCONST_1, 1));
		l.addInstruction(new SimpleInstruction(Opcode.SRETURN));
		l.addInstructionHandle(brtarget0);
		l.addInstruction(new ConstantPushInstruction(Opcode.SCONST_0, 0));
		l.addInstruction(new SimpleInstruction(Opcode.SRETURN));

		return l;
	}

	private static LightweightMethod addXZand1ifYnotnull() {
		LightweightMethod l = new LightweightMethod("javax.rtcbench.RTCBenchmark", "addXZand1ifYnotnull");
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

		l.addInstruction(new ArithmeticInstruction(Opcode.IADD));
		l.addInstruction(new BranchInstruction(Opcode.IFNULL, 0)).setBranchHandle(brtarget0);
		l.addInstruction(new ConstantPushInstruction(Opcode.ICONST_1, 1));
		l.addInstruction(new ArithmeticInstruction(Opcode.IADD));
		l.addInstructionHandle(brtarget0);
		l.addInstruction(new ArithmeticInstruction(Opcode.IADD));
		l.addInstruction(new SimpleInstruction(Opcode.IRETURN));

		return l;
	}

	private static LightweightMethod timesTenTestHighStackShort() {
		LightweightMethod l = new LightweightMethod("javax.rtcbench.RTCBenchmark", "timesTenTestHighStackShort");

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

		l.addInstruction(new StackInstruction(Opcode.IDUP));
		l.addInstruction(new StackInstruction(Opcode.IDUP));
		l.addInstruction(new StackInstruction(Opcode.IDUP));
		l.addInstruction(new StackInstruction(Opcode.IDUP));
		l.addInstruction(new StackInstruction(Opcode.IDUP));
		l.addInstruction(new StackInstruction(Opcode.IDUP));
		l.addInstruction(new StackInstruction(Opcode.IDUP));
		l.addInstruction(new StackInstruction(Opcode.IDUP));
		l.addInstruction(new StackInstruction(Opcode.IDUP));
		l.addInstruction(new ArithmeticInstruction(Opcode.SADD));
		l.addInstruction(new ArithmeticInstruction(Opcode.SADD));
		l.addInstruction(new ArithmeticInstruction(Opcode.SADD));
		l.addInstruction(new ArithmeticInstruction(Opcode.SADD));
		l.addInstruction(new ArithmeticInstruction(Opcode.SADD));
		l.addInstruction(new ArithmeticInstruction(Opcode.SADD));
		l.addInstruction(new ArithmeticInstruction(Opcode.SADD));
		l.addInstruction(new ArithmeticInstruction(Opcode.SADD));
		l.addInstruction(new ArithmeticInstruction(Opcode.SADD));
		l.addInstruction(new SimpleInstruction(Opcode.SRETURN));

		return l;
	}

	private static LightweightMethod timesTenTestHighStackInt() {
		LightweightMethod l = new LightweightMethod("javax.rtcbench.RTCBenchmark", "timesTenTestHighStackInt");

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

		l.addInstruction(new StackInstruction(Opcode.IDUP2));
		l.addInstruction(new StackInstruction(Opcode.IDUP2));
		l.addInstruction(new StackInstruction(Opcode.IDUP2));
		l.addInstruction(new StackInstruction(Opcode.IDUP2));
		l.addInstruction(new StackInstruction(Opcode.IDUP2));
		l.addInstruction(new StackInstruction(Opcode.IDUP2));
		l.addInstruction(new StackInstruction(Opcode.IDUP2));
		l.addInstruction(new StackInstruction(Opcode.IDUP2));
		l.addInstruction(new StackInstruction(Opcode.IDUP2));
		l.addInstruction(new ArithmeticInstruction(Opcode.IADD));
		l.addInstruction(new ArithmeticInstruction(Opcode.IADD));
		l.addInstruction(new ArithmeticInstruction(Opcode.IADD));
		l.addInstruction(new ArithmeticInstruction(Opcode.IADD));
		l.addInstruction(new ArithmeticInstruction(Opcode.IADD));
		l.addInstruction(new ArithmeticInstruction(Opcode.IADD));
		l.addInstruction(new ArithmeticInstruction(Opcode.IADD));
		l.addInstruction(new ArithmeticInstruction(Opcode.IADD));
		l.addInstruction(new ArithmeticInstruction(Opcode.IADD));
		l.addInstruction(new SimpleInstruction(Opcode.IRETURN));

		return l;
	}
}
