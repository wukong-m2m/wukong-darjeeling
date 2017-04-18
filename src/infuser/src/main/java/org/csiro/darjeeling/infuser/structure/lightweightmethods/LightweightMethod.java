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
	private BaseType returnType;
	private BaseType[] parameters;
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

	protected LightweightMethod(String className, String methodName, BaseType returnType, BaseType[] parameters) {
		this.className = className;
		this.methodName = methodName;
		// It would be nice if we could determine this from the CodeBlock, but we need the information
		// when we process the INVOKELIGHT instruction, and it's not guaranteed the Lightweight method
		// will already have been processed at that time.
		// Would be good to change this sometime 
		this.returnType = returnType;
		this.parameters = parameters;
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

	static {
		lightweightMethods = new ArrayList<LightweightMethod>();

        registerLightweightMethod(LightweightMethodImplementations.fft_FIX_MPY_lightweight());
        registerLightweightMethod(LightweightMethodImplementations.coremark_ee_isdigit_lightweight());
        registerLightweightMethod(LightweightMethodImplementations.testISWAP());
        registerLightweightMethod(LightweightMethodImplementations.testILOAD_ISTORE());
		registerLightweightMethod(LightweightMethodImplementations.isOddShort());
		registerLightweightMethod(LightweightMethodImplementations.isOddInt());
		registerLightweightMethod(LightweightMethodImplementations.isNull());
		registerLightweightMethod(LightweightMethodImplementations.timesTenTestHighStackShort());
		registerLightweightMethod(LightweightMethodImplementations.timesTenTestHighStackRef());
	}

 //    private static LightweightMethod fft_FIX_MPY_lightweight() {
 //        return new LightweightMethod("javax.rtcbench.RTCBenchmark", "FIX_MPY_lightweight", BaseType.Byte, new BaseType[] { BaseType.Byte, BaseType.Byte }) {
 //            @Override
 //            public void setupLightweightMethod() {

 //                // Java version
 //                // private static byte FIX_MPY(byte a, byte b)
 //                // {
 //                //     short c = (short)((short)((short)a * (short)b) >> 6);
 //                //     return (byte)((c >> 1) + ((byte)(c & 0x01)));
 //                // }

 //                // // Original (non-lightweight) JVM version:
 //                // sload(0)                      ;                                                             ; 0Short(Byte)                                                ;   ; 0,1
 //                // sload(1)                      ; 0Short(Byte)                                                ; 0Short(Byte),1Short(Byte)                                   ;   ; 1
 //                // smul                          ; 0Short(Byte),1Short(Byte)                                   ; 2Short                                                      ; G ; 
 //                // bspush                        ; 2Short                                                      ; 2Short,3Short(Byte)                                         ;   ; 
 //                // sshr                          ; 2Short,3Short(Byte)                                         ; 5Short                                                      ;   ; 
 //                // sstore(2)                     ; 5Short                                                      ;                                                             ;   ; 2
 //                // sload(2)                      ;                                                             ; 7Short                                                      ;  K; 2
 //                // sconst_1                      ; 7Short                                                      ; 7Short,8Short(Byte)                                         ;   ; 2
 //                // sshr                          ; 7Short,8Short(Byte)                                         ; 9Short                                                      ;   ; 2
 //                // sload(2)                      ; 9Short                                                      ; 9Short,10Short                                              ;   ; 2
 //                // sconst_1                      ; 9Short,10Short                                              ; 9Short,10Short,11Short(Byte)                                ;   ; 
 //                // sand                          ; 9Short,10Short,11Short(Byte)                                ; 9Short,12Short                                              ;   ; 
 //                // s2b                           ; 9Short,12Short                                              ; 9Short,13Byte                                               ;   ; 
 //                // sadd                          ; 9Short,13Byte                                               ; 14Short                                                     ;   ; 
 //                // s2b                           ; 14Short                                                     ; 15Byte                                                      ;   ; 
 //                // sreturn                       ; 15Byte                                                      ;                                                             ;   ; 

 //                // Lightweight version
 //                //                   // a, b
 //                // smul              // a*b
 //                // bspush 6          // a*b, 6
 //                // sshr              // c (=(a*b)>>6)
 //                // idup              // c, c
 //                // sconst_1          // c, c, 1
 //                // sshr              // c, c>>1
 //                // swap              // c>>1, c
 //                // sconst_1          // c>>1, c, 1
 //                // sand              // c>>1, c&1
 //                // sadd              // (c>>1 + c&1)
 //                // s2b               // (byte)(c>>1 + c&1)
 //                // sreturn

 //                addInstruction(new ArithmeticInstruction(Opcode.SMUL));
 //                addInstruction(new ImmediateBytePushInstruction(Opcode.BSPUSH, 6));
 //                addInstruction(new ArithmeticInstruction(Opcode.SSHR));
 //                addInstruction(new StackInstruction(Opcode.IDUP));
 //                addInstruction(new ConstantPushInstruction(Opcode.SCONST_1, 1));
 //                addInstruction(new ArithmeticInstruction(Opcode.SSHR));
 //                addInstruction(new StackInstruction(Opcode.ISWAP_X));
 //                addInstruction(new ConstantPushInstruction(Opcode.SCONST_1, 1));
 //                addInstruction(new ArithmeticInstruction(Opcode.SAND));
 //                addInstruction(new ArithmeticInstruction(Opcode.SADD));
 //                addInstruction(new ExplicitCastInstruction(Opcode.S2B));
 //                addInstruction(new SimpleInstruction(Opcode.SRETURN));

 //                return l;
 //            }
 //        };      
 //    }

	// private static LightweightMethod coremark_ee_isdigit_lightweight() {
	// 	return new LightweightMethod("javax.rtcbench.CoreState", "ee_isdigit_lightweight", BaseType.Short,  new BaseType[] { BaseType.Short }) {
	// 		@Override
	// 		public void setupLightweightMethod() {
	// 			InstructionHandle brtarget0 = new InstructionHandle(new BranchTargetInstruction(Opcode.BRTARGET));
	// 			InstructionHandle brtarget1 = new InstructionHandle(new BranchTargetInstruction(Opcode.BRTARGET));

	// 			// idup
	// 			// bspush 48
	// 			// if_scmplt brtarget 0
	// 			// bspush 57
	// 			// if_scmpgt brtarget 1
	// 			// sconst_1
	// 			// sreturn
	// 			// brtarget 0
	// 			// ipop // To clear the copy of the parameter. We won't need anymore.
	// 			// brtarget 1
	// 			// sconst_0
	// 			// sreturn

	// 			addInstruction(new StackInstruction(Opcode.IDUP));
	// 			addInstruction(new ImmediateBytePushInstruction(Opcode.BSPUSH, 48));
	// 			addInstruction(new BranchInstruction(Opcode.IF_SCMPLT, 0)).setBranchHandle(brtarget0);
	// 			addInstruction(new ImmediateBytePushInstruction(Opcode.BSPUSH, 57));
	// 			addInstruction(new BranchInstruction(Opcode.IF_SCMPGT, 0)).setBranchHandle(brtarget1);
	// 			addInstruction(new ConstantPushInstruction(Opcode.SCONST_1, 1));
	// 			addInstruction(new SimpleInstruction(Opcode.SRETURN));
	// 			addInstructionHandle(l, brtarget0);
	// 			addInstruction(new StackInstruction(Opcode.IPOP));				
	// 			addInstructionHandle(l, brtarget1);
	// 			addInstruction(new ConstantPushInstruction(Opcode.SCONST_0, 0));
	// 			addInstruction(new SimpleInstruction(Opcode.SRETURN));
	// 		}
	// 	};
	// }

	// private static LightweightMethod testISWAP() {
	// 	return new LightweightMethod("javax.rtcbench.RTCBenchmark", "testISWAP", BaseType.Int, new BaseType[] { BaseType.Short, BaseType.Int }) {
	// 		@Override
	// 		public void setupLightweightMethod() {

	// 			// iswap_x
	// 			// s2i
	// 			// iadd
	// 			// ireturn

 //                addInstruction(new StackInstruction(Opcode.ISWAP_X));
	// 			addInstruction(new ArithmeticInstruction(Opcode.S2I));
	// 			addInstruction(new ArithmeticInstruction(Opcode.IADD));
	// 			addInstruction(new SimpleInstruction(Opcode.IRETURN));
	// 		}
	// 	};
	// }
	// private static LightweightMethod testILOAD_ISTORE() {
	// 	return new LightweightMethod("javax.rtcbench.RTCBenchmark", "testILOAD_ISTORE", BaseType.Int, new BaseType[] { BaseType.Short, BaseType.Int }) {
	// 		@Override
	// 		public void setupLightweightMethod() {
	// 			LocalVariable i0 = addLocalVariable(0, BaseType.Int);

	// 			// istore_0
	// 			// s2i
	// 			// iload_0
	// 			// iadd
	// 			// ireturn

 //                addInstruction(new LoadStoreInstruction(Opcode.ISTORE, i0));
	// 			addInstruction(new ArithmeticInstruction(Opcode.S2I));
 //                addInstruction(new LoadStoreInstruction(Opcode.ILOAD, i0));
	// 			addInstruction(new ArithmeticInstruction(Opcode.IADD));
	// 			addInstruction(new SimpleInstruction(Opcode.IRETURN));
	// 		}
	// 	};
	// }
	// private static LightweightMethod isOddShort() {
	// 	return new LightweightMethod("javax.rtcbench.RTCBenchmark", "isOddShort",  BaseType.Short, new BaseType[] { BaseType.Short }) {
	// 		@Override
	// 		public void setupLightweightMethod() {
	// 			InstructionHandle brtarget0 = new InstructionHandle(new BranchTargetInstruction(Opcode.BRTARGET));

	// 			// sconst_1
	// 			// sand
	// 			// sreturn

	// 			addInstruction(new ConstantPushInstruction(Opcode.SCONST_1, 1));
	// 			addInstruction(new ArithmeticInstruction(Opcode.SAND));
	// 			addInstruction(new SimpleInstruction(Opcode.SRETURN));
	// 		}
	// 	};
	// }

	// private static LightweightMethod isOddInt() {
	// 	return new LightweightMethod("javax.rtcbench.RTCBenchmark", "isOddInt",  BaseType.Short, new BaseType[] { BaseType.Int }) {
	// 		@Override
	// 		public void setupLightweightMethod() {

	// 			// i2s
	// 			// sconst_1
	// 			// sand
	// 			// sreturn

	// 			addInstruction(new ExplicitCastInstruction(Opcode.I2S));
	// 			addInstruction(new ConstantPushInstruction(Opcode.SCONST_1, 1));
	// 			addInstruction(new ArithmeticInstruction(Opcode.SAND));
	// 			addInstruction(new SimpleInstruction(Opcode.SRETURN));
	// 		}
	// 	};
	// }

	// private static LightweightMethod isNull() {
	// 	return new LightweightMethod("javax.rtcbench.RTCBenchmark", "isNull",  BaseType.Short, new BaseType[] { BaseType.Ref }) {
	// 		@Override
	// 		public void setupLightweightMethod() {
	// 			InstructionHandle brtarget0 = new InstructionHandle(new BranchTargetInstruction(Opcode.BRTARGET));

	// 			// ifnull brtarget 0
	// 			// sconst_1
	// 			// sret
	// 			// brtarget 0
	// 			// sconst_0
	// 			// sret

	// 			addInstruction(new BranchInstruction(Opcode.IFNULL, 0)).setBranchHandle(brtarget0);
	// 			addInstruction(new ConstantPushInstruction(Opcode.SCONST_0, 0));
	// 			addInstruction(new SimpleInstruction(Opcode.SRETURN));
	// 			addInstructionHandle(l, brtarget0);
	// 			addInstruction(new ConstantPushInstruction(Opcode.SCONST_1, 1));
	// 			addInstruction(new SimpleInstruction(Opcode.SRETURN));
	// 		}
	// 	};
	// }

	// private static LightweightMethod timesTenTestHighStackShort() {
	// 	return new LightweightMethod("javax.rtcbench.RTCBenchmark", "timesTenTestHighStackShort",  BaseType.Short, new BaseType[] { BaseType.Short }) {
	// 		@Override
	// 		public void setupLightweightMethod() {

	// 			// test if the infuser handles the case where the lightweight method has a stack higher than the parameters,
	// 			// and higher than the calling method correctly. it needs to increase maxIntStack for the caller.

	// 			// idup
	// 			// idup
	// 			// idup
	// 			// idup
	// 			// idup
	// 			// idup
	// 			// idup
	// 			// idup
	// 			// idup
	// 			// sadd
	// 			// sadd
	// 			// sadd
	// 			// sadd
	// 			// sadd
	// 			// sadd
	// 			// sadd
	// 			// sadd
	// 			// sadd
	// 			// bspush 42
	// 			// sreturn

	// 			addInstruction(new StackInstruction(Opcode.IDUP));
	// 			addInstruction(new StackInstruction(Opcode.IDUP));
	// 			addInstruction(new StackInstruction(Opcode.IDUP));
	// 			addInstruction(new StackInstruction(Opcode.IDUP));
	// 			addInstruction(new StackInstruction(Opcode.IDUP));
	// 			addInstruction(new StackInstruction(Opcode.IDUP));
	// 			addInstruction(new StackInstruction(Opcode.IDUP));
	// 			addInstruction(new StackInstruction(Opcode.IDUP));
	// 			addInstruction(new StackInstruction(Opcode.IDUP));
	// 			addInstruction(new ArithmeticInstruction(Opcode.SADD));
	// 			addInstruction(new ArithmeticInstruction(Opcode.SADD));
	// 			addInstruction(new ArithmeticInstruction(Opcode.SADD));
	// 			addInstruction(new ArithmeticInstruction(Opcode.SADD));
	// 			addInstruction(new ArithmeticInstruction(Opcode.SADD));
	// 			addInstruction(new ArithmeticInstruction(Opcode.SADD));
	// 			addInstruction(new ArithmeticInstruction(Opcode.SADD));
	// 			addInstruction(new ArithmeticInstruction(Opcode.SADD));
	// 			addInstruction(new ArithmeticInstruction(Opcode.SADD));
	// 			addInstruction(new SimpleInstruction(Opcode.SRETURN));
	// 		}
	// 	};
	// }

	// private static LightweightMethod timesTenTestHighStackRef() {
	// 	return new LightweightMethod("javax.rtcbench.RTCBenchmark", "timesTenTestHighStackRef", BaseType.Short, new BaseType[] { BaseType.Ref }) {
	// 		@Override
	// 		public void setupLightweightMethod() {

	// 			// test if the infuser handles the case where the lightweight method has a stack higher than the parameters,
	// 			// and higher than the calling method correctly. it needs to increase maxIntStack for the caller.

	// 			// adup
	// 			// adup
	// 			// adup
	// 			// adup
	// 			// adup
	// 			// adup
	// 			// adup
	// 			// adup
	// 			// adup
	// 			// apop
	// 			// apop
	// 			// apop
	// 			// apop
	// 			// apop
	// 			// apop
	// 			// apop
	// 			// apop
	// 			// apop
	// 			// apop
	// 			// sreturn

	// 			addInstruction(new StackInstruction(Opcode.ADUP));
	// 			addInstruction(new StackInstruction(Opcode.ADUP));
	// 			addInstruction(new StackInstruction(Opcode.ADUP));
	// 			addInstruction(new StackInstruction(Opcode.ADUP));
	// 			addInstruction(new StackInstruction(Opcode.ADUP));
	// 			addInstruction(new StackInstruction(Opcode.ADUP));
	// 			addInstruction(new StackInstruction(Opcode.ADUP));
	// 			addInstruction(new StackInstruction(Opcode.ADUP));
	// 			addInstruction(new StackInstruction(Opcode.ADUP));
	// 			addInstruction(new ArithmeticInstruction(Opcode.APOP));
	// 			addInstruction(new ArithmeticInstruction(Opcode.APOP));
	// 			addInstruction(new ArithmeticInstruction(Opcode.APOP));
	// 			addInstruction(new ArithmeticInstruction(Opcode.APOP));
	// 			addInstruction(new ArithmeticInstruction(Opcode.APOP));
	// 			addInstruction(new ArithmeticInstruction(Opcode.APOP));
	// 			addInstruction(new ArithmeticInstruction(Opcode.APOP));
	// 			addInstruction(new ArithmeticInstruction(Opcode.APOP));
	// 			addInstruction(new ArithmeticInstruction(Opcode.APOP));
	// 			addInstruction(new ArithmeticInstruction(Opcode.APOP));
	// 			addInstruction(new ImmediateBytePushInstruction(Opcode.BSPUSH, 42));
	// 			addInstruction(new SimpleInstruction(Opcode.SRETURN));
	// 		}
	// 	};
	// }
}
