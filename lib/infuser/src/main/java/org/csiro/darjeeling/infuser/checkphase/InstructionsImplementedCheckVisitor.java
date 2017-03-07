/*
 * InstructionsImplementedCheckVisitor.java
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
 
package org.csiro.darjeeling.infuser.checkphase;

import org.apache.bcel.Constants;
import org.apache.bcel.classfile.Code;
import org.apache.bcel.classfile.LineNumberTable;
import org.apache.bcel.generic.Instruction;
import org.apache.bcel.generic.InstructionHandle;
import org.apache.bcel.generic.InstructionList;
import org.csiro.darjeeling.infuser.logging.Logging;
import org.csiro.darjeeling.infuser.structure.Element;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodImplementation;

public class InstructionsImplementedCheckVisitor extends CheckVisitor
{
	
	public InstructionsImplementedCheckVisitor()
	{
	}
	
	public void visit(InternalMethodImplementation element)
	{
		super.visit(element);

		Code code = element.getCode();
		if (code!=null) checkCode(code);
	}
	
	private void checkCode(Code code)
	{
		// get instruction list and annotate
		InstructionList instructionList = new InstructionList(code.getCode());
		
		LineNumberTable lineNumberTable = code.getLineNumberTable();
		
		for (InstructionHandle handle : instructionList.getInstructionHandles())
		{
			Instruction instruction = handle.getInstruction(); 
			switch (instruction.getOpcode())
			{
				case Constants.IADD:
				case Constants.ISUB:
				case Constants.IMUL:
				case Constants.IDIV:
				case Constants.INEG:
				case Constants.ISHR:
				case Constants.IUSHR:
				case Constants.ISHL:
				case Constants.IREM:
				case Constants.IAND:
				case Constants.IOR:
				case Constants.IXOR:
				case Constants.I2B:
				case Constants.I2C:
				case Constants.I2S:
				case Constants.IINC:
				case Constants.ICONST_M1:
				case Constants.ICONST_0:
				case Constants.ICONST_1:
				case Constants.ICONST_2:
				case Constants.ICONST_3:
				case Constants.ICONST_4:
				case Constants.ICONST_5:
				case Constants.BIPUSH:
				case Constants.SIPUSH:
				case Constants.LDC:
				case Constants.LDC_W:
				case Constants.ISTORE:
				case Constants.ISTORE_0:
				case Constants.ISTORE_1:
				case Constants.ISTORE_2:
				case Constants.ISTORE_3:
				case Constants.ILOAD:
				case Constants.ILOAD_0:
				case Constants.ILOAD_1:
				case Constants.ILOAD_2:
				case Constants.ILOAD_3:
				case Constants.ACONST_NULL:
				case Constants.ALOAD:
				case Constants.ALOAD_0:
				case Constants.ALOAD_1:
				case Constants.ALOAD_2:
				case Constants.ALOAD_3:
				case Constants.ASTORE:
				case Constants.ASTORE_0:
				case Constants.ASTORE_1:
				case Constants.ASTORE_2:
				case Constants.ASTORE_3:
				case Constants.POP:
				case Constants.POP2:
				case Constants.DUP:
				case Constants.DUP2:
				case Constants.DUP_X1:
				case Constants.DUP_X2:
				case Constants.GOTO:
				case Constants.IF_ICMPEQ:
				case Constants.IF_ICMPNE:
				case Constants.IF_ICMPLT:
				case Constants.IF_ICMPGE:
				case Constants.IF_ICMPGT:
				case Constants.IF_ICMPLE:
				case Constants.IFEQ:
				case Constants.IFNE:
				case Constants.IFLT:
				case Constants.IFGE:
				case Constants.IFGT:
				case Constants.IFLE:
				case Constants.IF_ACMPEQ:
				case Constants.IF_ACMPNE:
				case Constants.IFNULL:
				case Constants.IFNONNULL:
				case Constants.RETURN:
				case Constants.IRETURN:
				case Constants.ARETURN:
				case Constants.INVOKESTATIC:
				case Constants.INVOKESPECIAL:
				case Constants.INVOKEVIRTUAL:
				case Constants.INVOKEINTERFACE:
				case Constants.MONITORENTER:
				case Constants.MONITOREXIT:
				case Constants.NEW:
				case Constants.INSTANCEOF:
				case Constants.CHECKCAST:
				case Constants.NEWARRAY:
				case Constants.ANEWARRAY:
				case Constants.ARRAYLENGTH:
				case Constants.BASTORE:
				case Constants.CASTORE:
				case Constants.SASTORE:
				case Constants.IASTORE:
				case Constants.AASTORE:
				case Constants.BALOAD:
				case Constants.CALOAD:
				case Constants.SALOAD:
				case Constants.IALOAD:
				case Constants.AALOAD:
				case Constants.GETSTATIC:
				case Constants.PUTSTATIC:
				case Constants.GETFIELD:
				case Constants.PUTFIELD:
				case Constants.ATHROW:
				case Constants.TABLESWITCH:
				case Constants.LOOKUPSWITCH:

				case Constants.LCONST_0:
				case Constants.LCONST_1:
				case Constants.LLOAD:
				case Constants.LLOAD_0:
				case Constants.LLOAD_1:
				case Constants.LLOAD_2:
				case Constants.LLOAD_3:
				case Constants.LSTORE:
				case Constants.LSTORE_0:
				case Constants.LSTORE_1:
				case Constants.LSTORE_2:
				case Constants.LSTORE_3:
				case Constants.LALOAD:
				case Constants.LASTORE:
				case Constants.LADD:
				case Constants.LSUB:
				case Constants.LMUL:
				case Constants.LDIV:
				case Constants.LREM:
				case Constants.LNEG:
				case Constants.LSHL:
				case Constants.LSHR:
				case Constants.LUSHR:
				case Constants.LAND:
				case Constants.LOR:
				case Constants.LXOR:

				case Constants.LRETURN:

				case Constants.L2I:
				case Constants.I2L:

				case Constants.LCMP:

				case Constants.LDC2_W:
					
					// do nothing
					break;
				default:
					int line = 0;
					if (lineNumberTable!=null)
						line = lineNumberTable.getSourceLine(handle.getPosition());
					
					Logging.instance.error(line, String.format("Instruction %s not implemented", instruction.getName()));
			}
		}
		
	}

	@Override
	public void visit(Element element)
	{
	}

}
