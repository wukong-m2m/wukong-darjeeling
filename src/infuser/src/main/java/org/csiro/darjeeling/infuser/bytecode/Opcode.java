/*
 * Opcode.java
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

import java.util.Arrays;
import java.util.List;

import org.csiro.darjeeling.infuser.structure.BaseType;

/**
 * 
 * 
 * @author Niels Brouwers
 */
public enum Opcode
{
	
	
	NOP((short)0,"nop", null),
	SCONST_M1((short)1,"sconst_m1", BaseType.Short),
	SCONST_0((short)2,"sconst_0", BaseType.Short),
	SCONST_1((short)3,"sconst_1", BaseType.Short),
	SCONST_2((short)4,"sconst_2", BaseType.Short),
	SCONST_3((short)5,"sconst_3", BaseType.Short),
	SCONST_4((short)6,"sconst_4", BaseType.Short),
	SCONST_5((short)7,"sconst_5", BaseType.Short),
	ICONST_M1((short)8,"iconst_m1", BaseType.Int),
	ICONST_0((short)9,"iconst_0", BaseType.Int),
	ICONST_1((short)10,"iconst_1", BaseType.Int),
	ICONST_2((short)11,"iconst_2", BaseType.Int),
	ICONST_3((short)12,"iconst_3", BaseType.Int),
	ICONST_4((short)13,"iconst_4", BaseType.Int),
	ICONST_5((short)14,"iconst_5", BaseType.Int),
	ACONST_NULL((short)15,"aconst_null", BaseType.Ref),
	BSPUSH((short)16,"bspush", BaseType.Short),
	BIPUSH((short)17,"bipush", BaseType.Int),
	SSPUSH((short)18,"sspush", BaseType.Short),
	SIPUSH((short)19,"sipush", BaseType.Int),
	IIPUSH((short)20,"iipush", BaseType.Int),
	LDS((short)21,"lds", BaseType.Ref),
	SLOAD((short)22,"sload", BaseType.Short),
	SLOAD_0((short)23,"sload_0", BaseType.Short),
	SLOAD_1((short)24,"sload_1", BaseType.Short),
	SLOAD_2((short)25,"sload_2", BaseType.Short),
	SLOAD_3((short)26,"sload_3", BaseType.Short),
	ILOAD((short)27,"iload", BaseType.Int),
	ILOAD_0((short)28,"iload_0", BaseType.Int),
	ILOAD_1((short)29,"iload_1", BaseType.Int),
	ILOAD_2((short)30,"iload_2", BaseType.Int),
	ILOAD_3((short)31,"iload_3", BaseType.Int),
	ALOAD((short)32,"aload", BaseType.Ref),
	ALOAD_0((short)33,"aload_0", BaseType.Ref),
	ALOAD_1((short)34,"aload_1", BaseType.Ref),
	ALOAD_2((short)35,"aload_2", BaseType.Ref),
	ALOAD_3((short)36,"aload_3", BaseType.Ref),
	SSTORE((short)37,"sstore", null, BaseType.Short),
	SSTORE_0((short)38,"sstore_0", null, BaseType.Short),
	SSTORE_1((short)39,"sstore_1", null, BaseType.Short),
	SSTORE_2((short)40,"sstore_2", null, BaseType.Short),
	SSTORE_3((short)41,"sstore_3", null, BaseType.Short),
	ISTORE((short)42,"istore", null, BaseType.Int),
	ISTORE_0((short)43,"istore_0", null, BaseType.Int),
	ISTORE_1((short)44,"istore_1", null, BaseType.Int),
	ISTORE_2((short)45,"istore_2", null, BaseType.Int),
	ISTORE_3((short)46,"istore_3", null, BaseType.Int),
	ASTORE((short)47,"astore", null),
	ASTORE_0((short)48,"astore_0", null),
	ASTORE_1((short)49,"astore_1", null),
	ASTORE_2((short)50,"astore_2", null),
	ASTORE_3((short)51,"astore_3", null),
	BALOAD((short)52,"baload", BaseType.Byte, BaseType.Int),
	CALOAD((short)53,"caload", BaseType.Char, BaseType.Int),
	SALOAD((short)54,"saload", BaseType.Short, BaseType.Int),
	IALOAD((short)55,"iaload", BaseType.Int, BaseType.Int),
	AALOAD((short)56,"aaload", BaseType.Ref, BaseType.Int),
	BASTORE((short)57,"bastore", null, BaseType.Int, BaseType.Byte),
	CASTORE((short)58,"castore", null, BaseType.Int, BaseType.Char),
	SASTORE((short)59,"sastore", null, BaseType.Int, BaseType.Short),
	IASTORE((short)60,"iastore", null, BaseType.Int, BaseType.Int),
	AASTORE((short)61,"aastore", null, BaseType.Int, BaseType.Ref),
	IPOP((short)62,"ipop", null),
	IPOP2((short)63,"ipop2", null),
	IDUP((short)64,"idup", null),
	IDUP2((short)65,"idup2", null),
	IDUP_X1((short)66,"idup_x1", null),
	ISWAP_X((short)67,"iswap_x", null),
	APOP((short)68,"apop", null),
	APOP2((short)69,"apop2", null),
	ADUP((short)70,"adup", null),
	ADUP2((short)71,"adup2", null),
	ADUP_X1((short)72,"adup_x1", null),
	ADUP_X2((short)73,"adup_x2", null),
	ASWAP((short)74,"aswap", null),
	GETFIELD_B((short)75,"getfield_b", BaseType.Byte),
	GETFIELD_C((short)76,"getfield_c", BaseType.Char),
	GETFIELD_S((short)77,"getfield_s", BaseType.Short),
	GETFIELD_I((short)78,"getfield_i", BaseType.Int),
	GETFIELD_A((short)79,"getfield_a", BaseType.Ref),
	PUTFIELD_B((short)80,"putfield_b", null, BaseType.Byte),
	PUTFIELD_C((short)81,"putfield_c", null, BaseType.Char),
	PUTFIELD_S((short)82,"putfield_s", null, BaseType.Short),
	PUTFIELD_I((short)83,"putfield_i", null, BaseType.Int),
	PUTFIELD_A((short)84,"putfield_a", null),
	GETSTATIC_B((short)85,"getstatic_b", BaseType.Byte),
	GETSTATIC_C((short)86,"getstatic_c", BaseType.Char),
	GETSTATIC_S((short)87,"getstatic_s", BaseType.Short),
	GETSTATIC_I((short)88,"getstatic_i", BaseType.Int),
	GETSTATIC_A((short)89,"getstatic_a", BaseType.Ref),
	PUTSTATIC_B((short)90,"putstatic_b", null, BaseType.Byte),
	PUTSTATIC_C((short)91,"putstatic_c", null, BaseType.Char),
	PUTSTATIC_S((short)92,"putstatic_s", null, BaseType.Short),
	PUTSTATIC_I((short)93,"putstatic_i", null, BaseType.Int),
	PUTSTATIC_A((short)94,"putstatic_a", null),
	SADD((short)95,"sadd", BaseType.Short, BaseType.Short, BaseType.Short),
	SSUB((short)96,"ssub", BaseType.Short, BaseType.Short, BaseType.Short),
	SMUL((short)97,"smul", BaseType.Short, BaseType.Short, BaseType.Short),
	SDIV((short)98,"sdiv", BaseType.Short, BaseType.Short, BaseType.Short),
	SREM((short)99,"srem", BaseType.Short, BaseType.Short, BaseType.Short),
	SNEG((short)100,"sneg", BaseType.Short, BaseType.Short),
	SSHL((short)101,"sshl", BaseType.Short, BaseType.Short, BaseType.Short),
	SSHR((short)102,"sshr", BaseType.Short, BaseType.Short, BaseType.Short),
	SUSHR((short)103,"sushr", BaseType.Short, BaseType.Short, BaseType.Short),
	SAND((short)104,"sand", BaseType.Short, BaseType.Short, BaseType.Short),
	SOR((short)105,"sor", BaseType.Short, BaseType.Short, BaseType.Short),
	SXOR((short)106,"sxor", BaseType.Short, BaseType.Short, BaseType.Short),
	IADD((short)107,"iadd", BaseType.Int, BaseType.Int, BaseType.Int),
	ISUB((short)108,"isub", BaseType.Int, BaseType.Int, BaseType.Int),
	IMUL((short)109,"imul", BaseType.Int, BaseType.Int, BaseType.Int),
	IDIV((short)110,"idiv", BaseType.Int, BaseType.Int, BaseType.Int),
	IREM((short)111,"irem", BaseType.Int, BaseType.Int, BaseType.Int),
	INEG((short)112,"ineg", BaseType.Int, BaseType.Int),
	ISHL((short)113,"ishl", BaseType.Int, BaseType.Int, BaseType.Int),
	ISHR((short)114,"ishr", BaseType.Int, BaseType.Int, BaseType.Int),
	IUSHR((short)115,"iushr", BaseType.Int, BaseType.Int, BaseType.Short),
	IAND((short)116,"iand", BaseType.Int, BaseType.Int, BaseType.Int),
	IOR((short)117,"ior", BaseType.Int, BaseType.Int, BaseType.Int),
	IXOR((short)118,"ixor", BaseType.Int, BaseType.Int, BaseType.Int),
	BINC((short)119,"binc", null),
	SINC((short)120,"sinc", null),
	IINC((short)121,"iinc", null),
	S2B((short)122,"s2b", BaseType.Byte, BaseType.Short),
	S2I((short)123,"s2i", BaseType.Int, BaseType.Short),
	I2B((short)124,"i2b", BaseType.Byte, BaseType.Int),
	I2S((short)125,"i2s", BaseType.Short, BaseType.Int),
	IIFEQ((short)126,"iifeq", null, BaseType.Int),
	IIFNE((short)127,"iifne", null, BaseType.Int),
	IIFLT((short)128,"iiflt", null, BaseType.Int),
	IIFGE((short)129,"iifge", null, BaseType.Int),
	IIFGT((short)130,"iifgt", null, BaseType.Int),
	IIFLE((short)131,"iifle", null, BaseType.Int),
	IFNULL((short)132,"ifnull", null),
	IFNONNULL((short)133,"ifnonnull", null),
	IF_SCMPEQ((short)134,"if_scmpeq", null, BaseType.Short, BaseType.Short), // if_scmp{eq, ne, lt, ge, gt}
	IF_SCMPNE((short)135,"if_scmpne", null, BaseType.Short, BaseType.Short),
	IF_SCMPLT((short)136,"if_scmplt", null, BaseType.Short, BaseType.Short),
	IF_SCMPGE((short)137,"if_scmpge", null, BaseType.Short, BaseType.Short),
	IF_SCMPGT((short)138,"if_scmpgt", null, BaseType.Short, BaseType.Short),
	IF_SCMPLE((short)139,"if_scmple", null, BaseType.Short, BaseType.Short),
	IF_ICMPEQ((short)140,"if_icmpeq", null, BaseType.Int, BaseType.Int),
	IF_ICMPNE((short)141,"if_icmpne", null, BaseType.Int, BaseType.Int),
	IF_ICMPLT((short)142,"if_icmplt", null, BaseType.Int, BaseType.Int),
	IF_ICMPGE((short)143,"if_icmpge", null, BaseType.Int, BaseType.Int),
	IF_ICMPGT((short)144,"if_icmpgt", null, BaseType.Int, BaseType.Int),
	IF_ICMPLE((short)145,"if_icmple", null, BaseType.Int, BaseType.Int),
	IF_ACMPEQ((short)146,"if_acmpeq", null),
	IF_ACMPNE((short)147,"if_acmpne", null),
	GOTO((short)148,"goto", null),
	// GOTO_W((short)149,"goto_w"),
	TABLESWITCH((short)150,"tableswitch", null, BaseType.Int),
	LOOKUPSWITCH((short)151,"lookupswitch", null, BaseType.Int),
	SRETURN((short)152,"sreturn", null, BaseType.Short),
	IRETURN((short)153,"ireturn", null, BaseType.Int),
	ARETURN((short)154,"areturn", null),
	RETURN((short)155,"return", null),
	INVOKEVIRTUAL((short)156,"invokevirtual", null),
	INVOKESPECIAL((short)157,"invokespecial", null),
	INVOKESTATIC((short)158,"invokestatic", null),
	INVOKEINTERFACE((short)159,"invokeinterface", null),
	NEW((short)160,"new", BaseType.Ref),
	NEWARRAY((short)161,"newarray", BaseType.Ref, BaseType.Short),
	ANEWARRAY((short)162,"anewarray", BaseType.Ref, BaseType.Short),
	ARRAYLENGTH((short)163,"arraylength", BaseType.Short),
	ATHROW((short)164,"athrow", null),
	CHECKCAST((short)165,"checkcast", BaseType.Ref),
	INSTANCEOF((short)166,"instanceof", BaseType.Boolean),
	MONITORENTER((short)167,"monitorenter", null),
	MONITOREXIT((short)168,"monitorexit", null),	
	IDUP_X2((short)169,"idup_x2", null),
	IINC_W((short)170,"iinc_w", null),
	SINC_W((short)171,"sinc_w", null),
	I2C((short)172,"i2c", BaseType.Char, BaseType.Int),
	S2C((short)173,"s2c", BaseType.Char, BaseType.Short),
	B2C((short)174,"b2c", BaseType.Char, BaseType.Byte),
	IDUP_X((short)175,"idup_x", null),

	SIFEQ((short)176,"sifeq", null, BaseType.Short),
	SIFNE((short)177,"sifne", null, BaseType.Short),
	SIFLT((short)178,"siflt", null, BaseType.Short),
	SIFGE((short)179,"sifge", null, BaseType.Short),
	SIFGT((short)180,"sifgt", null, BaseType.Short),
	SIFLE((short)181,"sifle", null, BaseType.Short),

	LCONST_0((short)182,"lconst_0", BaseType.Long),
	LCONST_1((short)183,"lconst_1", BaseType.Long),
	LLOAD((short)184,"lload", BaseType.Long),
	LLOAD_0((short)185,"lload_0", BaseType.Long),
	LLOAD_1((short)186,"lload_1", BaseType.Long),
	LLOAD_2((short)187,"lload_2", BaseType.Long),
	LLOAD_3((short)188,"lload_3", BaseType.Long),
	LLPUSH((short)189,"llpush", BaseType.Long),
	LSTORE((short)190,"lstore", null, BaseType.Long),
	LSTORE_0((short)191,"lstore_0", null, BaseType.Long),
	LSTORE_1((short)192,"lstore_1", null, BaseType.Long),
	LSTORE_2((short)193,"lstore_2", null, BaseType.Long),
	LSTORE_3((short)194,"lstore_3", null, BaseType.Long),
	LALOAD((short)195,"laload", BaseType.Long, BaseType.Int),
	LASTORE((short)196,"lastore", null, BaseType.Int, BaseType.Long),
	GETFIELD_L((short)197,"getfield_l", BaseType.Long),
	PUTFIELD_L((short)198,"putfield_l", null, BaseType.Long),
	GETSTATIC_L((short)199,"getstatic_l", BaseType.Long),
	PUTSTATIC_L((short)200,"putstatic_l", null, BaseType.Long),

	LADD((short)201,"ladd", BaseType.Long, BaseType.Long, BaseType.Long),
	LSUB((short)202,"lsub", BaseType.Long, BaseType.Long, BaseType.Long),
	LMUL((short)203,"lmul", BaseType.Long, BaseType.Long, BaseType.Long),
	LDIV((short)204,"ldiv", BaseType.Long, BaseType.Long, BaseType.Long),
	LREM((short)205,"lrem", BaseType.Long, BaseType.Long, BaseType.Long),
	LNEG((short)206,"lneg", BaseType.Long, BaseType.Long),
	LSHL((short)207,"lshl", BaseType.Long, BaseType.Long, BaseType.Long),
	LSHR((short)208,"lshr", BaseType.Long, BaseType.Long, BaseType.Long),
	LUSHR((short)209,"lushr", BaseType.Long, BaseType.Long, BaseType.Short),
	LAND((short)210,"land", BaseType.Long, BaseType.Long, BaseType.Long),
	LOR((short)211,"lor", BaseType.Long, BaseType.Long, BaseType.Long),
	LXOR((short)212,"lxor", BaseType.Long, BaseType.Long, BaseType.Long),

	LRETURN((short)213,"lreturn", null, BaseType.Long),
	
	L2I((short)214,"l2i", BaseType.Int, BaseType.Long),
	L2S((short)215,"l2s", BaseType.Short, BaseType.Long),
	I2L((short)216,"i2l", BaseType.Long, BaseType.Int),
	S2L((short)217,"s2l", BaseType.Long, BaseType.Short),

	LCMP((short)218,"lcmp", BaseType.Short, BaseType.Long, BaseType.Long),
	
	// this is a dummy placeholder opcode, will not appear in the final output
	S2S((short)-1,"s2s", BaseType.Short, BaseType.Short)
	;
	
	// Defines the group of conditional branch instructions. Membership testing on this group is used in the isConditionalBranch method.
	private static List<Opcode> conditionalBranchInstructions = 
		Arrays.asList(new Opcode[] {
				SIFEQ, SIFNE, SIFLT, SIFGE, SIFGT, SIFLE,
				IIFEQ, IIFNE, IIFLT, IIFGE, IIFGT, IIFLE,
				IFNULL, IFNONNULL,
				IF_SCMPEQ, IF_SCMPNE, IF_SCMPLT, IF_SCMPGE, IF_SCMPGT, IF_SCMPLE,
				IF_ICMPEQ, IF_ICMPNE, IF_ICMPLT, IF_ICMPGE, IF_ICMPGT, IF_ICMPLE,
				IF_ACMPEQ, IF_ACMPNE,
				});

	// Defines the group of switch instructions. Membership testing on this group is used in the isSwitch method.
	private static List<Opcode> switchInstructions = Arrays.asList(new Opcode[] { TABLESWITCH, LOOKUPSWITCH });

	// Defines the group of integer load/store instructions. Membership testing on this group is used in the isIntLoadStore method.
	private static List<Opcode> intLoadStoreInstructions = 
		Arrays.asList(new Opcode[] {
				SLOAD, SLOAD_0, SLOAD_1, SLOAD_2, SLOAD_3, 
				SSTORE, SSTORE_0, SSTORE_1, SSTORE_2, SSTORE_3,
				ILOAD, ILOAD_0, ILOAD_1, ILOAD_2, ILOAD_3,
				ISTORE, ISTORE_0, ISTORE_1, ISTORE_2, ISTORE_3,
				LLOAD, LLOAD_0, LLOAD_1, LLOAD_2, LLOAD_3,
				LSTORE, LSTORE_0, LSTORE_1, LSTORE_2, LSTORE_3,
				});
	
	// Defines the group of reference load/store instructions. Membership testing on this group is used in the isIntLoadStore method.
	private static List<Opcode> refLoadStoreInstructions = 
		Arrays.asList(new Opcode[] { ALOAD, ALOAD_0, ALOAD_1, ALOAD_2, ALOAD_3, ASTORE, ASTORE_0, ASTORE_1, ASTORE_2, ASTORE_3 });
	
	// Defines the group of local variable store instructions. Membership testing on this group is used in the isStore method.
	private static List<Opcode> storeInstructions = 
		Arrays.asList(new Opcode[] {
				SSTORE, SSTORE_0, SSTORE_1, SSTORE_2, SSTORE_3, 
				ISTORE, ISTORE_0, ISTORE_1, ISTORE_2, ISTORE_3, 
				LSTORE, LSTORE_0, LSTORE_1, LSTORE_2, LSTORE_3, 
				ASTORE, ASTORE_0, ASTORE_1, ASTORE_2, ASTORE_3 });

	// Defines the group of local variable load instructions. Membership testing on this group is used in the isStore method.
	private static List<Opcode> loadInstructions = 
		Arrays.asList(new Opcode[] {
				SLOAD, SLOAD_0, SLOAD_1, SLOAD_2, SLOAD_3,
				ILOAD, ILOAD_0, ILOAD_1, ILOAD_2, ILOAD_3,
				LLOAD, LLOAD_0, LLOAD_1, LLOAD_2, LLOAD_3,
				ALOAD, ALOAD_0, ALOAD_1, ALOAD_2, ALOAD_3 });
	
	// Defines the group of return instructions. Membership testing on this group is used in the isReturn method.
	private static List<Opcode> returnInstructions = 
		Arrays.asList(new Opcode[] { RETURN, SRETURN, IRETURN, LRETURN, ARETURN });
	
	// Defines the group of return instructions. Membership testing on this group is used in the isInvoke method.
	private static List<Opcode> invokeInstructions = 
		Arrays.asList(new Opcode[] { INVOKEINTERFACE, INVOKESPECIAL, INVOKESTATIC, INVOKEVIRTUAL });

	private short opcode;
	private String name;
	private BaseType outputType;
	private BaseType operandTypes[];
	
	private Opcode(short opcode, String name, BaseType outputType, BaseType ... operandTypes)
	{
		this.opcode = opcode;
		this.name = name;
		this.operandTypes = operandTypes;
		this.outputType = outputType;
	}
	
	/**
	 * Checks if the opcode is a switch instruction (tableswitch, lookupswitch)
	 * @return true if the opcode is a switch instruction, false otherwise
	 */
	public boolean isSwitch()
	{
		return switchInstructions.contains(this); 
	}
	
	/**
	 * Checks if the opcode is a branch instruction (both conditional and unconditional branches, switches)
	 * @return true if the opcode is a branch instruction, false otherwise
	 */
	public boolean isBranch()
	{
		return conditionalBranchInstructions.contains(this) || switchInstructions.contains(this) || this==GOTO;
	}
	
	/**
	 * Checks if the opcode is a conditional branch instruction 
	 * @return true if the opcode is a conditional branch instruction, false otherwise
	 */
	public boolean isConditionalBranch()
	{
		return conditionalBranchInstructions.contains(this);
	}
	
	/**
	 * Checks if the opcode is an unconditional branch instruction (GOTO) 
	 * @return true if the opcode is an unconditional branch instruction, false otherwise
	 */
	public boolean isUnConditionalBranch()
	{
		return this==GOTO;
	}
	
	/**
	 * Checks if the opcode is an integer type local variable load/store instruction (ISTORE, SSTORE, ILOAD, SLOAD, and friends) 
	 * @return true if the opcode is an integer type local variable load/store instruction
	 */
	public boolean isIntLoadStoreInstruction()
	{
		return intLoadStoreInstructions.contains(this);
	}
	
	/**
	 * Checks if the opcode is a reference type local variable load/store instruction (ASTORE, ALOAD, and friends) 
	 * @return true if the opcode is an integer type local variable load/store instruction
	 */
	public boolean isRefLoadStoreInstruction()
	{
		return refLoadStoreInstructions.contains(this);
	}
	
	/**
	 * Checks if the opcode is a local variable store instruction (SSTORE, ISTORE, ASTORE, and friends) 
	 * @return true if the opcode is a local variable store instruction
	 */
	public boolean isStoreInstruction()
	{
		return storeInstructions.contains(this);
	}
	
	/**
	 * Checks if the opcode is a local variable load instruction (SLOAD, ILOAD, ALOAD, and friends) 
	 * @return true if the opcode is a local variable load instruction
	 */
	public boolean isLoadInstruction()
	{
		return loadInstructions.contains(this);
	}
	
	/**
	 * Checks if the opcode is a return instruction (IRETURN, SRETURN, ARETURN, RETURN) 
	 * @return true if the opcode is a return instruction
	 */
	public boolean isReturn()
	{
		return returnInstructions.contains(this);
	}

	/**
	 * Checks if the opcode is a throw instruction (ATHROW) 
	 * @return true if the opcode is a throw instruction
	 */
	public boolean isThrow()
	{
		return this==ATHROW;
	}
	
	/**
	 * Checks if the opcode is an invoke instruction (INVOKEINTERFACE, INVOKESPECIAL, INVOKESTATIC, INVOKEVIRTUAL) 
	 * @return true if the opcode is an invoke instruction
	 */
	public boolean isInvoke()
	{
		return invokeInstructions.contains(this);
	}
	
	/**
	 * Checks if the opcode is a virtual invoke instruction (INVOKEINTERFACE, INVOKESPECIAL, INVOKEVIRTUAL) 
	 * @return true if the opcode is a virtual invoke instruction
	 */
	public boolean isVirtualInvoke()
	{
		return invokeInstructions.contains(this) && this!=INVOKESTATIC;
	}
	
	/**
	 * @return opcode number
	 */
	public short getOpcode()
	{
		return opcode;
	}
	
	/**
	 * @return opcode name
	 */
	public String getName()
	{
		return name;
	}
	
	/**
	 * Gets the expected operand types. For instance, an SSTORE instruction expects its input type to be of width Short.  
	 * @return expected operand types
	 */
	public BaseType[] getOperandTypes()
	{
		return operandTypes;
	}
	
	public BaseType getOutputType()
	{
		return outputType;
	}

}
