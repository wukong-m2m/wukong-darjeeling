/*
 * BCELInstructionAdapter.java
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
 
package org.csiro.darjeeling.infuser.bytecode;

import static org.apache.bcel.Constants.*;

import java.util.HashMap;

import org.apache.bcel.Constants;
import org.apache.bcel.generic.ArrayType;
import org.apache.bcel.generic.BasicType;
import org.apache.bcel.generic.ConstantPoolGen;
import org.apache.bcel.generic.GETFIELD;
import org.apache.bcel.generic.IINC;
import org.apache.bcel.generic.InstructionHandle;
import org.apache.bcel.generic.InvokeInstruction;
import org.apache.bcel.generic.LOOKUPSWITCH;
import org.apache.bcel.generic.NEWARRAY;
import org.apache.bcel.generic.ObjectType;
import org.apache.bcel.generic.PUTFIELD;
import org.apache.bcel.generic.TABLESWITCH;
import org.apache.bcel.generic.Type;
import org.csiro.darjeeling.infuser.bytecode.instructions.ArithmeticInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.BranchInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.ConstantPushInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.ExplicitCastInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.FieldInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.ImmediateBytePushInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.ImmediateIntPushInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.ImmediateLongPushInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.ImmediateShortPushInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.IncreaseInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.IntegerConditionalBranchInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.LoadStoreInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.LocalIdInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.LongCompareInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.LookupSwitchInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.NewArrayInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.SimpleInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.StackInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.StaticInvokeInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.TableSwitchInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.VirtualInvokeInstruction;
import org.csiro.darjeeling.infuser.bytecode.instructions.WideIncreaseInstruction;
import org.csiro.darjeeling.infuser.structure.BaseType;
import org.csiro.darjeeling.infuser.structure.GlobalId;
import org.csiro.darjeeling.infuser.structure.LocalId;
import org.csiro.darjeeling.infuser.structure.elements.AbstractClassDefinition;
import org.csiro.darjeeling.infuser.structure.elements.AbstractField;
import org.csiro.darjeeling.infuser.structure.elements.AbstractInfusion;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodDefinition;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodImplementation;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalInfusion;

/**
 * Adapter that converts BCEL instructions to corresponding Darjeeling instructions. Note that there are some
 * instructions, such as stack manipulation instructions, that act as placeholders and that will be converted 
 * to their final form during bytecode transformation.   
 * <p>
 * The first phase conversion from Java bytecode to Darjeeling bytecode, each method implementation in a class 
 * file is parsed by BCEL and the instructions are then converted to corresponding Darjeeling instructions by
 * a BCELInstructionAdapter and put into a CodeBlock object. Since the generated Instruction objects are context 
 * sensitive in terms of both the Infusion and CodeBlock they are in, a new adapter has to be constructed for 
 * each CodeBlock that is created.
 * 
 * @author Niels Brouwers
 *
 */
public class BCELInstructionAdapter
{
	
	private InternalInfusion infusion;
	private ConstantPoolGen constantPoolGen;
	private CodeBlock codeBlock;
	
	/**
	 * Maps between Java and Darjeeling bytecodes
	 */
	private static HashMap<Short, Opcode> opcodeMap;
	
	static
	{
		opcodeMap = new HashMap<Short, Opcode>();
		opcodeMap.put(Constants.NOP, Opcode.NOP);
		opcodeMap.put(Constants.ICONST_M1, Opcode.ICONST_M1);
		opcodeMap.put(Constants.ICONST_0, Opcode.ICONST_0);
		opcodeMap.put(Constants.ICONST_1, Opcode.ICONST_1);
		opcodeMap.put(Constants.ICONST_2, Opcode.ICONST_2);
		opcodeMap.put(Constants.ICONST_3, Opcode.ICONST_3);
		opcodeMap.put(Constants.ICONST_4, Opcode.ICONST_4);
		opcodeMap.put(Constants.ICONST_5, Opcode.ICONST_5);
		opcodeMap.put(Constants.ACONST_NULL, Opcode.ACONST_NULL);
		opcodeMap.put(Constants.BIPUSH, Opcode.BIPUSH);
		opcodeMap.put(Constants.SIPUSH, Opcode.SIPUSH);
		opcodeMap.put(Constants.ILOAD, Opcode.ILOAD);
		opcodeMap.put(Constants.ILOAD_0, Opcode.ILOAD);
		opcodeMap.put(Constants.ILOAD_1, Opcode.ILOAD);
		opcodeMap.put(Constants.ILOAD_2, Opcode.ILOAD);
		opcodeMap.put(Constants.ILOAD_3, Opcode.ILOAD);
		opcodeMap.put(Constants.ALOAD, Opcode.ALOAD);
		opcodeMap.put(Constants.ALOAD_0, Opcode.ALOAD);
		opcodeMap.put(Constants.ALOAD_1, Opcode.ALOAD);
		opcodeMap.put(Constants.ALOAD_2, Opcode.ALOAD);
		opcodeMap.put(Constants.ALOAD_3, Opcode.ALOAD);
		opcodeMap.put(Constants.ISTORE, Opcode.ISTORE);
		opcodeMap.put(Constants.ISTORE_0, Opcode.ISTORE);
		opcodeMap.put(Constants.ISTORE_1, Opcode.ISTORE);
		opcodeMap.put(Constants.ISTORE_2, Opcode.ISTORE);
		opcodeMap.put(Constants.ISTORE_3, Opcode.ISTORE);
		opcodeMap.put(Constants.ASTORE, Opcode.ASTORE);
		opcodeMap.put(Constants.ASTORE_0, Opcode.ASTORE);
		opcodeMap.put(Constants.ASTORE_1, Opcode.ASTORE);
		opcodeMap.put(Constants.ASTORE_2, Opcode.ASTORE);
		opcodeMap.put(Constants.ASTORE_3, Opcode.ASTORE);
		opcodeMap.put(Constants.BALOAD, Opcode.BALOAD);
		opcodeMap.put(Constants.CALOAD, Opcode.CALOAD);
		opcodeMap.put(Constants.SALOAD, Opcode.SALOAD);
		opcodeMap.put(Constants.IALOAD, Opcode.IALOAD);
		opcodeMap.put(Constants.LALOAD, Opcode.LALOAD);
		opcodeMap.put(Constants.AALOAD, Opcode.AALOAD);
		opcodeMap.put(Constants.BASTORE, Opcode.BASTORE);
		opcodeMap.put(Constants.CASTORE, Opcode.CASTORE);
		opcodeMap.put(Constants.SASTORE, Opcode.SASTORE);
		opcodeMap.put(Constants.IASTORE, Opcode.IASTORE);
		opcodeMap.put(Constants.LASTORE, Opcode.LASTORE);
		opcodeMap.put(Constants.AASTORE, Opcode.AASTORE);
		opcodeMap.put(Constants.IADD, Opcode.IADD);
		opcodeMap.put(Constants.ISUB, Opcode.ISUB);
		opcodeMap.put(Constants.IMUL, Opcode.IMUL);
		opcodeMap.put(Constants.IDIV, Opcode.IDIV);
		opcodeMap.put(Constants.IREM, Opcode.IREM);
		opcodeMap.put(Constants.INEG, Opcode.INEG);
		opcodeMap.put(Constants.ISHL, Opcode.ISHL);
		opcodeMap.put(Constants.ISHR, Opcode.ISHR);
		opcodeMap.put(Constants.IUSHR, Opcode.IUSHR);
		opcodeMap.put(Constants.IAND, Opcode.IAND);
		opcodeMap.put(Constants.IOR, Opcode.IOR);
		opcodeMap.put(Constants.IXOR, Opcode.IXOR);
		opcodeMap.put(Constants.IINC, Opcode.IINC);
		opcodeMap.put(Constants.I2C, Opcode.I2C);
		opcodeMap.put(Constants.I2B, Opcode.I2B);
		opcodeMap.put(Constants.I2S, Opcode.I2S);
		opcodeMap.put(Constants.IFEQ, Opcode.IIFEQ);
		opcodeMap.put(Constants.IFNE, Opcode.IIFNE);
		opcodeMap.put(Constants.IFLT, Opcode.IIFLT);
		opcodeMap.put(Constants.IFGE, Opcode.IIFGE);
		opcodeMap.put(Constants.IFGT, Opcode.IIFGT);
		opcodeMap.put(Constants.IFLE, Opcode.IIFLE);
		opcodeMap.put(Constants.IFNULL, Opcode.IFNULL);
		opcodeMap.put(Constants.IFNONNULL, Opcode.IFNONNULL);
		opcodeMap.put(Constants.IF_ICMPEQ, Opcode.IF_ICMPEQ);
		opcodeMap.put(Constants.IF_ICMPNE, Opcode.IF_ICMPNE);
		opcodeMap.put(Constants.IF_ICMPLT, Opcode.IF_ICMPLT);
		opcodeMap.put(Constants.IF_ICMPGE, Opcode.IF_ICMPGE);
		opcodeMap.put(Constants.IF_ICMPGT, Opcode.IF_ICMPGT);
		opcodeMap.put(Constants.IF_ICMPLE, Opcode.IF_ICMPLE);
		opcodeMap.put(Constants.IF_ACMPEQ, Opcode.IF_ACMPEQ);
		opcodeMap.put(Constants.IF_ACMPNE, Opcode.IF_ACMPNE);
		opcodeMap.put(Constants.GOTO, Opcode.GOTO);
		opcodeMap.put(Constants.TABLESWITCH, Opcode.TABLESWITCH);
		opcodeMap.put(Constants.LOOKUPSWITCH, Opcode.LOOKUPSWITCH);
		opcodeMap.put(Constants.IRETURN, Opcode.IRETURN);
		opcodeMap.put(Constants.ARETURN, Opcode.ARETURN);
		opcodeMap.put(Constants.RETURN, Opcode.RETURN);
		opcodeMap.put(Constants.INVOKEVIRTUAL, Opcode.INVOKEVIRTUAL);
		opcodeMap.put(Constants.INVOKESPECIAL, Opcode.INVOKESPECIAL);
		opcodeMap.put(Constants.INVOKESTATIC, Opcode.INVOKESTATIC);
		opcodeMap.put(Constants.INVOKEINTERFACE, Opcode.INVOKEINTERFACE);
		opcodeMap.put(Constants.NEW, Opcode.NEW);
		opcodeMap.put(Constants.NEWARRAY, Opcode.NEWARRAY);
		opcodeMap.put(Constants.ANEWARRAY, Opcode.ANEWARRAY);
		opcodeMap.put(Constants.ARRAYLENGTH, Opcode.ARRAYLENGTH);
		opcodeMap.put(Constants.ATHROW, Opcode.ATHROW);
		opcodeMap.put(Constants.CHECKCAST, Opcode.CHECKCAST);
		opcodeMap.put(Constants.INSTANCEOF, Opcode.INSTANCEOF);
		opcodeMap.put(Constants.MONITORENTER, Opcode.MONITORENTER);
		opcodeMap.put(Constants.MONITOREXIT, Opcode.MONITOREXIT);

		opcodeMap.put(Constants.LCONST_0, Opcode.LCONST_0);
		opcodeMap.put(Constants.LCONST_1, Opcode.LCONST_1);
		opcodeMap.put(Constants.LLOAD, Opcode.LLOAD);
		opcodeMap.put(Constants.LLOAD_0, Opcode.LLOAD);
		opcodeMap.put(Constants.LLOAD_1, Opcode.LLOAD);
		opcodeMap.put(Constants.LLOAD_2, Opcode.LLOAD);
		opcodeMap.put(Constants.LLOAD_3, Opcode.LLOAD);
		opcodeMap.put(Constants.LSTORE, Opcode.LSTORE);
		opcodeMap.put(Constants.LSTORE_0, Opcode.LSTORE);
		opcodeMap.put(Constants.LSTORE_1, Opcode.LSTORE);
		opcodeMap.put(Constants.LSTORE_2, Opcode.LSTORE);
		opcodeMap.put(Constants.LSTORE_3, Opcode.LSTORE);

		opcodeMap.put(Constants.LADD, Opcode.LADD);
		opcodeMap.put(Constants.LSUB, Opcode.LSUB);
		opcodeMap.put(Constants.LMUL, Opcode.LMUL);
		opcodeMap.put(Constants.LDIV, Opcode.LDIV);
		opcodeMap.put(Constants.LREM, Opcode.LREM);
		opcodeMap.put(Constants.LNEG, Opcode.LNEG);
		opcodeMap.put(Constants.LSHL, Opcode.LSHL);
		opcodeMap.put(Constants.LSHR, Opcode.LSHR);
		opcodeMap.put(Constants.LUSHR, Opcode.LUSHR);
		opcodeMap.put(Constants.LAND, Opcode.LAND);
		opcodeMap.put(Constants.LOR, Opcode.LOR);
		opcodeMap.put(Constants.LXOR, Opcode.LXOR);
		opcodeMap.put(Constants.LRETURN, Opcode.LRETURN);
		opcodeMap.put(Constants.L2I, Opcode.L2I);
		opcodeMap.put(Constants.I2L, Opcode.I2L);

		opcodeMap.put(Constants.LCMP, Opcode.LCMP);
	}
	
	/**
	 * 
	 * @param infusion InternalInfusion that will contain the generated instructions. 
	 * @param constantPoolGen BCEL ConstantPoolGen object needed for resolving things like method and class names, signatures etc
	 * @param codeBlock the CodeBlock object that the generated instructions will belong to. This is needed for establishing direct 
	 * links to LocalVariable objects in the code blocks.
	 */
	public BCELInstructionAdapter(InternalInfusion infusion, ConstantPoolGen constantPoolGen, CodeBlock codeBlock)
	{
		this.infusion = infusion;
		this.constantPoolGen = constantPoolGen;		
		this.codeBlock = codeBlock;
	}
	
	/**
	 * @return the Infusion context of this adapter
	 */
	public AbstractInfusion getInfusion()
	{
		return infusion;
	}
	
	/**
	 * @return BCEL ConstantPoolGen
	 */
	public ConstantPoolGen getConstantPoolGen()
	{
		return constantPoolGen;
	}
	
	/**
	 * Provides a mapping between BCEL's representation of JVM instructions and 
	 * Darjeeling instructions.
	 * @param instruction BCEL Instruction 
	 * @return a valid Darjeeling Instruction 
	 */
	private Opcode map(org.apache.bcel.generic.Instruction instruction)
	{
		short opcode = instruction.getOpcode();
		if (!opcodeMap.keySet().contains(opcode))
				throw new IllegalArgumentException("Unable to map illegal/unsupported instruction: " + instruction);
		else
			return opcodeMap.get(opcode);
	}
	
	/**
	 * Performs a method lookup for a given BCEL invoke instruction by method definition (signature, method name),
	 * and parent class. If a method implementation is found in the Infusion the GlobalId of method is returned.
	 * If the method could not be found an IllegalStateException is thrown.  
	 * @param invoke BCEL InvokeInstruction
	 * @return a GlobalId to the method implementation 
	 */
	private AbstractMethodImplementation lookupMethodImplementation(InvokeInstruction invoke)
	{
		String methodName = invoke.getMethodName(constantPoolGen); 
		String methodSignature = invoke.getSignature(constantPoolGen); 
		String parentClass = invoke.getReferenceType(constantPoolGen).toString(); 
		
		// lookup method
		AbstractMethodImplementation methodImpl = infusion.lookupMethodImplemention(methodName,	methodSignature, parentClass);
		
		// make sure the method implementation was found
		if (methodImpl == null)
			throw new IllegalStateException(String.format("Method not found: %s:%s in class %s", methodSignature, methodName, parentClass));
		
		return methodImpl;
	}
	
	/**
	 * Performs a method lookup for a given BCEL invoke instruction by method definition (signature, method name),
	 * and parent class. If a method implementation is found in the Infusion the GlobalId of method is returned.
	 * If the method could not be found an IllegalStateException is thrown.  
	 * @param invoke BCEL InvokeInstruction
	 * @return a GlobalId to the method implementation 
	 */
	private AbstractMethodDefinition lookupMethodDefinition(InvokeInstruction invoke)
	{
		String methodName = invoke.getMethodName(constantPoolGen); 
		String methodSignature = invoke.getSignature(constantPoolGen); 
		
		// lookup method
		AbstractMethodDefinition methodDefinition = infusion.lookupMethodDefinition(methodName, methodSignature);
		
		// make sure the method implementation was found
		if (methodDefinition == null)
			throw new IllegalStateException(String.format("Method definition not found: %s:%s", methodSignature, methodName));
		
		return methodDefinition;
	}
	
	/**
	 * Takes a BCEL FieldInstruction and finds the corresponding static field in the infusion.  
	 * @param instruction BCEL FieldInstruction
	 * @return an AbstractField instance in the Infusion
	 */
	private AbstractField lookupStaticField(org.apache.bcel.generic.FieldInstruction instruction)
	{
		// get field information from the instruction
		String className = instruction.getReferenceType(constantPoolGen).toString();
		String fieldName = instruction.getFieldName(constantPoolGen);
		String descriptor = instruction.getSignature(constantPoolGen);
		
		// find a static field in our static field list that 
		// corresponds with this field
		AbstractField field = infusion.lookupStaticField(className, fieldName, descriptor);
		
		// make sure the field was found
		if (field==null)
			throw new IllegalStateException(String.format("Static field not found: %s %s.%s", descriptor, className, fieldName));
		
		return field;
	}	
	
	/**
	 * Takes a BCEL FieldInstruction and finds the corresponding field in the infusion.  
	 * @param instruction BCEL FieldInstruction
	 * @return an AbstractField instance in the Infusion
	 */
	private AbstractField lookupField(org.apache.bcel.generic.FieldInstruction instruction)
	{
		String fieldName = instruction.getFieldName(constantPoolGen);
		String className = instruction.getReferenceType(constantPoolGen).toString();
		
		// get containing class
		AbstractClassDefinition classDef = infusion.lookupClassByName(className);
		
		// if the classDef is null, scream bloody murder
		if (classDef==null)
			throw new IllegalStateException("Class not found " + className);

		// find the field in the class
		AbstractField field = classDef.getFieldByName(fieldName);
		
		// if the field does not exist in the class, again, scream bloody murder
		if (field==null)
			throw new IllegalStateException(String.format("Field not found: %s in %s", fieldName, className));

		return field;
	}
	
	/**
	 * Locates a Class in the infusion by class name and returns its LocalId. If the class is not found an 
	 * IllegalStateException is thrown
	 * @param className
	 * @return LocalId of the class
	 */
	private LocalId getClassID(String className)
	{
		// retrieve the global_class_id from the class list
		AbstractClassDefinition classDef = infusion.lookupClassByName(className);

		if (classDef==null)
			throw new IllegalStateException("Class not found: " + className);
		
		// get global class id
		return classDef.getGlobalId().resolve(infusion);
			
	}
	
	private LocalId getClassID(ObjectType type)
	{
		return getClassID(type.getClassName());
	}
	
	private LocalId getClassID(ArrayType type)
	{
		Type elementType = type.getElementType();
		
		// array of objects
		if (elementType instanceof ObjectType)
			return getClassID((ObjectType)elementType);
		
		// array of integer types
		if (elementType instanceof BasicType)
			return LocalId.numArray(BaseType.fromBCELType((BasicType)elementType));
		
		throw new IllegalStateException("Unhandled Array Type");
		
	}
	
	private LocalId getClassID(Type type)
	{
		// handle array types
		if (type instanceof ArrayType)
			return getClassID((ArrayType)type);
		
		// handle object types
		if (type instanceof ObjectType)
			return getClassID((ObjectType)type);
		
		throw new IllegalStateException("Unhandled type in instanceof/checkcast");
	}
	
	/**
	 * This code maps a BCEL instruction onto a darjeeling instruction.
	 * @param instruction
	 * @return
	 */
	public Instruction fromBCELInstruction(org.apache.bcel.generic.Instruction instruction)
	{
		// precondition
		if (instruction==null)
			throw new IllegalArgumentException("instruction argument is null");

		Instruction ret = null;
		LocalId localId;
		org.apache.bcel.generic.InvokeInstruction invoke;
		org.apache.bcel.generic.ConstantPushInstruction push;
		org.apache.bcel.generic.BranchInstruction branch;
		AbstractMethodDefinition methodDefinition;
		AbstractMethodImplementation methodImplementation;
		AbstractField field;
		BaseType type;
		Type ttype;
		String className;
		InstructionHandle[] targets;
		int[] targetAdresses;
		Object value;
		
		switch(instruction.getOpcode())
		{
			// branch instructions
			case IFEQ:
			case IFNE:
			case IFLT:
			case IFGE:
			case IFGT:
			case IFLE:
			case IFNULL:
			case IFNONNULL:
			case IF_ACMPEQ:
			case IF_ACMPNE:
				// Get the branch address from the BCEL instruction. Due to size mismatches between
				// the JVM and DVM instructionset these offsets will not be correct. They will
				// be used for establishing links between instruction handles, and the new correct
				// addresses will be calculated later on.
				branch = (org.apache.bcel.generic.BranchInstruction)instruction;
				ret = new BranchInstruction(map(instruction), branch.getTarget().getPosition());
				break;

			case IF_ICMPEQ:
			case IF_ICMPNE:
			case IF_ICMPLT:
			case IF_ICMPGE:
			case IF_ICMPGT:
			case IF_ICMPLE:
				// Get the branch address from the BCEL instruction. Due to size mismatches between
				// the JVM and DVM instructionset these offsets will not be correct. They will
				// be used for establishing links between instruction handles, and the new correct
				// addresses will be calculated later on.
				branch = (org.apache.bcel.generic.BranchInstruction)instruction;
				ret = new IntegerConditionalBranchInstruction(map(instruction), branch.getTarget().getPosition());
				break;

			case GOTO:
				branch = (org.apache.bcel.generic.BranchInstruction)instruction;
				ret = new BranchInstruction(map(instruction), branch.getTarget().getPosition());
				break;
				
			// static invoke instructions
			case INVOKESPECIAL:
			case INVOKESTATIC:
				invoke = (org.apache.bcel.generic.InvokeInstruction)instruction;
				methodImplementation = lookupMethodImplementation(invoke);
				ret = new StaticInvokeInstruction(map(instruction), methodImplementation.getGlobalId().resolve(infusion), methodImplementation.getMethodDefinition());
				break;
				
			// virtual invoke instructions
			case INVOKEVIRTUAL:
			case INVOKEINTERFACE:
                invoke = (org.apache.bcel.generic.InvokeInstruction)instruction;

                // create a method def to lookup the method
                methodDefinition = lookupMethodDefinition(invoke);
				ret = new VirtualInvokeInstruction(map(instruction), methodDefinition.getGlobalId().resolve(infusion), methodDefinition);
				break;
				
			// Load constant, results in IIPUSH for integer and LDS for string
			case LDC:
			case LDC_W:
				org.apache.bcel.generic.LDC ldc = (org.apache.bcel.generic.LDC)instruction;
				value = ldc.getValue(constantPoolGen);
				if (value instanceof Integer)
				{
					ret = new ImmediateIntPushInstruction(Opcode.IIPUSH, ((Integer)value).intValue()); 
				} else if (value instanceof String)
				{
					String stringConstant = (java.lang.String)value;
					GlobalId id = infusion.lookupString(stringConstant);
					if (id==null) id = infusion.getStringTable().addString(stringConstant);
					
					ret = new LocalIdInstruction(Opcode.LDS, id.resolve(infusion) );
				} else
					throw new IllegalStateException("Unsupported type for LDC: " + value.getClass());
				break;

			case LDC2_W:
				org.apache.bcel.generic.LDC2_W ldc2 = (org.apache.bcel.generic.LDC2_W)instruction;
				value = ldc2.getValue(constantPoolGen);
				if (value instanceof Long)
				{
					ret = new ImmediateLongPushInstruction(Opcode.LLPUSH, ((Long)value).longValue());
				} else
					throw new IllegalStateException("Unsupported type for LDC: " + value.getClass());
				break;

			// Immediate value push instructions
			case BIPUSH:
				push = (org.apache.bcel.generic.ConstantPushInstruction)instruction;
				ret = new ImmediateBytePushInstruction(Opcode.BIPUSH, push.getValue().byteValue());
				break;
				
			case SIPUSH:
				push = (org.apache.bcel.generic.ConstantPushInstruction)instruction;
				ret = new ImmediateShortPushInstruction(Opcode.SIPUSH, push.getValue().shortValue());
				break;
			
			// constant pushes 
			case ICONST_M1: ret = new ConstantPushInstruction(Opcode.ICONST_M1, -1); break;
			case ICONST_0: ret = new ConstantPushInstruction(Opcode.ICONST_0, 0); break;
			case ICONST_1: ret = new ConstantPushInstruction(Opcode.ICONST_1, 1); break;
			case ICONST_2: ret = new ConstantPushInstruction(Opcode.ICONST_2, 2); break;
			case ICONST_3: ret = new ConstantPushInstruction(Opcode.ICONST_3, 3); break;
			case ICONST_4: ret = new ConstantPushInstruction(Opcode.ICONST_4, 4); break;
			case ICONST_5: ret = new ConstantPushInstruction(Opcode.ICONST_5, 5); break;
			case LCONST_0: ret = new ConstantPushInstruction(Opcode.LCONST_0, 0); break;
			case LCONST_1: ret = new ConstantPushInstruction(Opcode.LCONST_1, 1); break;
			
			// local variable instructions with an index 
			case ILOAD:
			case ILOAD_0:
			case ILOAD_1:
			case ILOAD_2:
			case ILOAD_3:
			case LLOAD:
			case LLOAD_0:
			case LLOAD_1:
			case LLOAD_2:
			case LLOAD_3:
			case ALOAD:
			case ALOAD_0:
			case ALOAD_1:
			case ALOAD_2:
			case ALOAD_3:
			case ISTORE:
			case ISTORE_0:
			case ISTORE_1:
			case ISTORE_2:
			case ISTORE_3:
			case LSTORE:
			case LSTORE_0:
			case LSTORE_1:
			case LSTORE_2:
			case LSTORE_3:
			case ASTORE:
			case ASTORE_0:
			case ASTORE_1:
			case ASTORE_2:
			case ASTORE_3:
				org.apache.bcel.generic.LocalVariableInstruction lvi = (org.apache.bcel.generic.LocalVariableInstruction)instruction; 
				ret = new LoadStoreInstruction(map(instruction), codeBlock.getLocalVariable(lvi.getIndex()));
				break;
				
			// field instructions
			case PUTFIELD:
				field = lookupField((PUTFIELD)instruction);
				type = AbstractField.classify(field.getDescriptor());
				switch (type)
				{
					case Boolean:
					case Byte: ret = new FieldInstruction(Opcode.PUTFIELD_B, field.getOffset()); break;
					case Char: ret = new FieldInstruction(Opcode.PUTFIELD_C, field.getOffset()); break;
					case Short: ret = new FieldInstruction(Opcode.PUTFIELD_S, field.getOffset()); break;
					case Int: ret = new FieldInstruction(Opcode.PUTFIELD_I, field.getOffset()); break;
					case Long: ret = new FieldInstruction(Opcode.PUTFIELD_L, field.getOffset()); break;
					case Ref: ret = new FieldInstruction(Opcode.PUTFIELD_A, field.getOffset()); break;
					default:
						throw new IllegalStateException("Unsupported type for putfield");
				}
				break;
			case GETFIELD:
				field = lookupField((GETFIELD)instruction);
				type = AbstractField.classify(field.getDescriptor());
				switch (type)
				{
					case Boolean:
					case Byte: ret = new FieldInstruction(Opcode.GETFIELD_B, field.getOffset()); break;
					case Char: ret = new FieldInstruction(Opcode.GETFIELD_C, field.getOffset()); break;
					case Short: ret = new FieldInstruction(Opcode.GETFIELD_S, field.getOffset()); break;
					case Int: ret = new FieldInstruction(Opcode.GETFIELD_I, field.getOffset()); break;
					case Long: ret = new FieldInstruction(Opcode.GETFIELD_L, field.getOffset()); break;
					case Ref: ret = new FieldInstruction(Opcode.GETFIELD_A, field.getOffset()); break;
					default:
						throw new IllegalStateException("Unsupported type for getfield");
				}
				break;
				
			// static fields
			case PUTSTATIC:
				field = lookupStaticField((org.apache.bcel.generic.FieldInstruction)instruction);
				localId = field.getGlobalId().resolve(infusion);
				type = AbstractField.classify(field.getDescriptor());
				
				if (field.getConstantValue()!=null)
					throw new IllegalStateException("PUTSTATIC on a final static field");
				
				switch (type)
				{
					case Boolean:
					case Byte: ret = new LocalIdInstruction(Opcode.PUTSTATIC_B, localId); break;
					case Char: ret = new LocalIdInstruction(Opcode.PUTSTATIC_C, localId); break;
					case Short: ret = new LocalIdInstruction(Opcode.PUTSTATIC_S, localId); break;
					case Int: ret = new LocalIdInstruction(Opcode.PUTSTATIC_I, localId); break;
					case Long: ret = new LocalIdInstruction(Opcode.PUTSTATIC_L, localId); break;
					case Ref: ret = new LocalIdInstruction(Opcode.PUTSTATIC_A, localId); break;
					default:
						throw new IllegalStateException("Unsupported type for putstatic");
				}
				break;
			case GETSTATIC:
				
				field = lookupStaticField((org.apache.bcel.generic.FieldInstruction)instruction);
				localId = field.getGlobalId().resolve(infusion);
				type = AbstractField.classify(field.getDescriptor());

				// If the field is final, we should convert the instruction to a constant push instruction.
				// This saves ram by moving the constant to program memory, effectively treating
				// final static fields as defines.
				if (field.getConstantValue()!=null)
					System.out.println("WARNING: final static field is not being converted to a constant");
					
				switch (type)
				{
					case Boolean:
					case Byte: ret = new LocalIdInstruction(Opcode.GETSTATIC_B, localId); break;
					case Char: ret = new LocalIdInstruction(Opcode.GETSTATIC_C, localId); break;
					case Short: ret = new LocalIdInstruction(Opcode.GETSTATIC_S, localId); break;
					case Int: ret = new LocalIdInstruction(Opcode.GETSTATIC_I, localId); break;
					case Long: ret = new LocalIdInstruction(Opcode.GETSTATIC_L, localId); break;
					case Ref: ret = new LocalIdInstruction(Opcode.GETSTATIC_A, localId); break;
					default:
						throw new IllegalStateException("Unsupported type for getstatic");
				}
				
				break;
			
			// new
			case NEW:
				className = ((org.apache.bcel.generic.NEW)instruction).getLoadClassType(constantPoolGen).toString();
				ret = new LocalIdInstruction(Opcode.NEW, getClassID(className));
				break;
			
			// instanceof
			case INSTANCEOF:
				ttype = ((org.apache.bcel.generic.INSTANCEOF)instruction).getType(constantPoolGen);
				ret = new LocalIdInstruction(Opcode.INSTANCEOF, getClassID(ttype));
				break;
				
			// checkcast
			case CHECKCAST:
				ttype = ((org.apache.bcel.generic.CHECKCAST)instruction).getType(constantPoolGen);
				ret = new LocalIdInstruction(Opcode.CHECKCAST, getClassID(ttype));
				break;
				
			// anewarray
			case ANEWARRAY:
				className = ((org.apache.bcel.generic.ANEWARRAY)instruction).getLoadClassType(constantPoolGen).toString();
				ret = new LocalIdInstruction(Opcode.ANEWARRAY, getClassID(className));
				break;
				
			// newarray
			case NEWARRAY:
				NEWARRAY newArray = (NEWARRAY)instruction;
				ret = new NewArrayInstruction(Opcode.NEWARRAY, newArray.getTypecode());
				break;
				
			// stack operations
			case POP: ret = new StackInstruction(Opcode.IPOP); break;
 			case POP2: ret = new StackInstruction(Opcode.IPOP2); break;
			case DUP: ret = new StackInstruction(Opcode.IDUP); break;
			case DUP2: ret = new StackInstruction(Opcode.IDUP2); break;
			case DUP_X1: ret = new StackInstruction(Opcode.IDUP_X1); break;
 			case DUP_X2: ret = new StackInstruction(Opcode.IDUP_X2); break;
 			case SWAP: ret = new StackInstruction(Opcode.ISWAP_X); break;
				
			// direct local variable increase
 			case IINC:
 				IINC iinc = (IINC)instruction;
 				int increment = iinc.getIncrement();
 				if (((byte)increment)==increment)
 	 				ret = new IncreaseInstruction(Opcode.IINC, codeBlock.getLocalVariable(iinc.getIndex()), iinc.getIncrement());
 				else
 	 				ret = new WideIncreaseInstruction(Opcode.IINC_W, codeBlock.getLocalVariable(iinc.getIndex()), iinc.getIncrement());
 				break;
 				
 			// tableswitch instruction
			case TABLESWITCH:
				TABLESWITCH tableSwitch = (TABLESWITCH)instruction;

				int match[] = tableSwitch.getMatchs();
				int low = (match.length > 0) ? match[0] : 0;
		        int high = (match.length > 0) ? match[match.length - 1] : 0;
				targets = tableSwitch.getTargets();
				targetAdresses = new int[targets.length];
				for (int i=0; i<targets.length; i++) targetAdresses[i] = targets[i].getPosition();				
				ret = new TableSwitchInstruction(Opcode.TABLESWITCH, tableSwitch.getTarget().getPosition(), low, high, targetAdresses);
				break;

 			// lookup instruction
			case LOOKUPSWITCH:
				LOOKUPSWITCH lookupSwitch = (LOOKUPSWITCH)instruction;

				targets = lookupSwitch.getTargets();
				targetAdresses = new int[targets.length];
				for (int i=0; i<targets.length; i++) targetAdresses[i] = targets[i].getPosition();
				
				ret = new LookupSwitchInstruction(Opcode.LOOKUPSWITCH, lookupSwitch.getTarget().getPosition(), lookupSwitch.getMatchs(), targetAdresses);
				break;
				
			// arithmetic instructions
			case IADD:
			case ISUB:
			case IMUL:
			case IDIV:
			case IREM:
			case INEG:
			case ISHL:
			case ISHR:
			case IUSHR:
			case IAND:
			case IOR:
			case IXOR:
			case LADD:
			case LSUB:
			case LMUL:
			case LDIV:
			case LREM:
			case LNEG:
			case LSHL:
			case LSHR:
			case LUSHR:
			case LAND:
			case LOR:
			case LXOR:
				ret = new ArithmeticInstruction(map(instruction));
				break;
				
			// explicit casts
			case I2C:
			case I2B:
			case I2S:
			case L2I:
			case I2L:
				ret = new ExplicitCastInstruction(map(instruction));
				break;
				
			case LCMP:
				ret = new LongCompareInstruction(map(instruction));
				break;

			// simple 1-byte instructions
			case NOP:
			case ACONST_NULL:

			case BALOAD:
			case CALOAD:
			case SALOAD:
			case IALOAD:
			case LALOAD:
			case AALOAD:
			case BASTORE:
			case CASTORE:
			case SASTORE:
			case IASTORE:
			case LASTORE:
			case AASTORE:
			
			case IRETURN:
			case LRETURN:
			case ARETURN:
			case RETURN:

			case ARRAYLENGTH:
			case ATHROW:
			case MONITORENTER:
			case MONITOREXIT:
				ret = new SimpleInstruction(map(instruction));
				break;
				
			// unsupported, will not implement
			default:
				throw new IllegalArgumentException("Cannot map BCEL instruction onto a darjeeling instruction: " + instruction);
		}
		
		return ret;
	}

}
