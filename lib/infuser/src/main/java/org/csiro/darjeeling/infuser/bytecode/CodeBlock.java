/*
 * CodeBlock.java
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

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.ArrayList;

import org.apache.bcel.classfile.Code;
import org.apache.bcel.classfile.CodeException;
import org.apache.bcel.classfile.ConstantClass;
import org.apache.bcel.generic.ConstantPoolGen;
import org.csiro.darjeeling.infuser.bytecode.transformations.AnalyseTypes;
import org.csiro.darjeeling.infuser.bytecode.transformations.CalculateMaxStack;
import org.csiro.darjeeling.infuser.bytecode.transformations.InsertExplicitCasts;
import org.csiro.darjeeling.infuser.bytecode.transformations.OptimizeByteCode;
import org.csiro.darjeeling.infuser.bytecode.transformations.ReMapLocalVariables;
import org.csiro.darjeeling.infuser.bytecode.transformations.ReplaceStackInstructions;
import org.csiro.darjeeling.infuser.structure.BaseType;
import org.csiro.darjeeling.infuser.structure.LocalId;
import org.csiro.darjeeling.infuser.structure.elements.AbstractClassDefinition;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalInfusion;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodImplementation;

/**
 * 
 * The CodeBlock object contains the code of method implementations. A MethodImplementation object
 * has either zero or one CodeBlock objects.
 * <p>
 * Aside from storing instructions the CodeBlock class performs byte code analysis and transformation 
 * through CodeBlockTransformation implementations. A factory method is provided to construct a CodeBlock 
 * from a BCEL Code object.
 * <p>
 * A code block contains instructions, local variables, and exception handlers. Local variables
 * are indexed by their original JVM slot numbers and get re-mapped at the end of the
 * transformation phase. 
 * 
 * @author Niels Brouwers
 *
 */
public class CodeBlock
{
	
	// List of InstructionHandles
	private InstructionList instructions;
	
	// Exception handlers
	private ArrayList<ExceptionHandler> exceptionHandlers;
	
	// Local variables
	private ArrayList<LocalVariable> localVariables;
	
	// The maximum number of slots needed for local variables and stack
	// These are initialised to whatever is in the Java class file and
	// get recalculated as the code block is transformed. The max locals
	// is only used in some of the initial steps, the max locals is
	// used after transformation is finished and holds the maximum
	// number of 16-bit slots on the operand stack.
	private int maxLocals, maxStack;
	
	// Local variables are split into two groups, reference types and integer
	// types. These variables hold how many of each there are in this code
	// block. They replace the Java maxLocals. 
	private int referenceLocalVariableCount, integerLocalVariableCount;
	
	// The method implementation this code block belongs to.
	// Used mostly to obtain things like the parameter list which is needed 
	// for bootstrapping type inference.
	private InternalMethodImplementation methodImplementation;
	
	/**
	 * Constructs a new, empty code block.
	 */
	private CodeBlock()
	{
		instructions = new InstructionList();
		localVariables = new ArrayList<LocalVariable>();
		exceptionHandlers = new ArrayList<ExceptionHandler>();
	}
	
	/**
	 * @return the exception handlers in this code block.
	 */
	public ExceptionHandler[] getExceptionHandlers()
	{
		ExceptionHandler ret[] = new ExceptionHandler[exceptionHandlers.size()];
		exceptionHandlers.toArray(ret);
		return ret;
	}
	
	/**
	 * @return the number of local variables that are of the reference type 
	 */
	public int getReferenceLocalVariableCount()
	{
		return referenceLocalVariableCount;
	}
	
	/**
	 * @return the number of local variables that are of the integer type 
	 */
	public int getIntegerLocalVariableCount()
	{
		return integerLocalVariableCount;
	}
	
	/**
	 * @param the number of local variables that are of the reference type 
	 */
	public void setReferenceLocalVariableCount(int referenceLocalVariableCount)
	{
		this.referenceLocalVariableCount = referenceLocalVariableCount;
	}
	
	/**
	 * @param the number of local variables that are of the integer type 
	 */
	public void setIntegerLocalVariableCount(int integerLocalVariableCount)
	{
		this.integerLocalVariableCount = integerLocalVariableCount;
	}
	
	/**
	 * @param index
	 * @return returns the local variable at the given slot index
	 */
	public LocalVariable getLocalVariable(int index)
	{
		return localVariables.get(index);
	}
	
	/**
	 * @return the local variables list. 
	 */
	public ArrayList<LocalVariable> getLocalVariables()
	{
		return localVariables;
	}
	
	/**
	 * @return the number of local variables
	 */
	public int getLocalVariableCount()
	{
		return localVariables.size();
	}
	
	/**
	 * Creates a new CodeBlock from an existing BCEL Code object. Code is transformed and optimised.
	 * @param code a BCEL Code object
	 * @param methodDefinition an AbstractMethodDefinition object describing the method
	 * @param infusion an infusion context
	 * @param cpg a BCEL ConstantPoolGen object 
	 * @return a new CodeBlock object containing valid DVM byte code that is semantically equivalent to the input JVM byte code
	 */
	public static CodeBlock fromCode(Code code, InternalMethodImplementation methodImplementation, InternalInfusion infusion, ConstantPoolGen cpg)
	{
		CodeBlock ret = new CodeBlock();

		ret.maxLocals = code.getMaxLocals();
		ret.maxStack = code.getMaxStack();
		ret.methodImplementation = methodImplementation;

		boolean isStatic = methodImplementation.isStatic();
		
		// create LocalVariable objects for each of the local variables
		for (int i=0; i<code.getMaxLocals(); i++)
		{
			LocalVariable localVariable = new LocalVariable(i);
			ret.localVariables.add(localVariable);
		}
		
		// If the method is not static (virtual), we can infer that the first local variable is a 
		// reference type (this)
		if (!isStatic)
			ret.localVariables.get(0).setType(BaseType.Ref);
		
		// We know the types of the parameters, so we can set these right away
		// TODO refactor
		BaseType[] parameterTypes = methodImplementation.getMethodDefinition().getArgumentTypes();
		int pos = 0;
		for (int i=0; i<parameterTypes.length; i++)
		{
			LocalVariable localVariable = ret.localVariables.get((isStatic?0:1)+pos); 
			localVariable.setType(parameterTypes[i]);
			pos += parameterTypes[i].isLongSized()?2:1;
		}
		
		// the first n local variables are parameters that are passed from the caller, so they are
		// parameters
		for (int i=0; i<parameterTypes.length+(isStatic?0:1); i++)
			ret.localVariables.get(i).setIsParameter(true);		
		
		// construct an adapter class that converts BCEL instructions to Darjeeling DVM instructions
		BCELInstructionAdapter adapter = new BCELInstructionAdapter(infusion, cpg, ret);
		
		// build a list of instruction handles from a BCEL instruction list
		// note that the original PC's are copied from the BCEL handles, even though
		// some darjeeling instructions have a different size. This is done so that
		// branch handles and exception handlers can be resolved easily.
		// The correct PC's are calculated at the end of this method.
		org.apache.bcel.generic.InstructionList instructionList = new org.apache.bcel.generic.InstructionList(code.getCode());
		for (org.apache.bcel.generic.InstructionHandle handle : instructionList.getInstructionHandles())
		{
			Instruction instruction = adapter.fromBCELInstruction(handle.getInstruction());
			InstructionHandle newHandle = new InstructionHandle(instruction);
			ret.instructions.addInstructionHandle(newHandle);
			newHandle.setPc(handle.getPosition());
		}
		
		// now that we have a list of handles and their original PC's, get the
		// exception handlers from the BCEL code object and convert them
		CodeException exceptions[] = code.getExceptionTable();
		if (exceptions!=null)
		{
			for (CodeException exception : exceptions)
			{
				// get instruction handles for the start/end of the catch area, and the handler itself
				InstructionHandle start = ret.instructions.getHandleByPc(exception.getStartPC());
				InstructionHandle end = ret.instructions.getHandleByPc(exception.getEndPC());
				InstructionHandle handler = ret.instructions.getHandleByPc(exception.getHandlerPC());

				LocalId catchType;
				
				if (exception.getCatchType()==0)
				{
					// The value (255,255), signals that the catch block should catch all objects
					catchType = new LocalId(255,255);
				} else
				{
					// painful process of getting the class name of this try/catch block
					ConstantClass classType = (ConstantClass)code.getConstantPool().getConstant(exception.getCatchType());
					String className = code.getConstantPool().getConstantString(exception.getCatchType(), classType.getTag());
					className = className.replaceAll("\\/",".");
					
					// resolve the class to get a global ID
					AbstractClassDefinition classDef = infusion.lookupClassByName(className);
					LocalId classId = classDef.getGlobalId().resolve(infusion);
					
					// write exception handler catch type ID
					catchType = new LocalId(classId.getInfusionId(), classId.getLocalId());
				}
				
				ret.exceptionHandlers.add(new ExceptionHandler(start, end, handler, catchType));
			}
		}

		// thread states, creating incoming and outgoing links between instruction handles
		ret.instructions.threadStates();
		
		// fix the branch addresses in the branch instructions
		ret.instructions.fixBranchAddresses();
		
		// perform type analysis
		new AnalyseTypes(ret).transform();

		new OptimizeByteCode(ret).transform();
		ret.instructions.reThreadStates();
		ret.instructions.fixBranchAddresses();
			
		// add casts where needed
		new InsertExplicitCasts(ret).transform();
		
		ret.instructions.reThreadStates();
		ret.instructions.fixBranchAddresses();
		new AnalyseTypes(ret).transform();
		
		// calculate maximum stack usage
		new CalculateMaxStack(ret).transform();

		// replace stack instructions like DUP, DUP2, POP, etc with the appropriate ones
		new ReplaceStackInstructions(ret).transform();
		
		ret.instructions.reThreadStates();
		
		// assign indices to the local variables
		new ReMapLocalVariables(ret).transform();

		// fix the branch addresses in the branch instructions
		ret.instructions.fixBranchAddresses();
		
		return ret;
	}
	
	public InstructionList getInstructions()
	{
		return instructions;
	}
	
	public int getMaxLocals()
	{
		return maxLocals;
	}
	
	public int getMaxStack()
	{
		return maxStack;
	}
	
	public void setMaxStack(int maxStack)
	{
		this.maxStack = maxStack;
	}
	
	public InternalMethodImplementation getMethodImplementation()
	{
		return methodImplementation;
	}
	
	/**
	 * Serialises the code block to a byte array.
	 * @return this code block as a byte array.
	 */
	public byte[] toByteArray()
	{
		ByteArrayOutputStream outStream = new ByteArrayOutputStream();
		DataOutputStream dataOutputStream = new DataOutputStream(outStream);
		
		try {
			for (InstructionHandle handle : instructions.getInstructionHandles())
				handle.getInstruction().dump(dataOutputStream);
			dataOutputStream.close();
			outStream.close();
		} catch (IOException ex)
		{
			throw new RuntimeException(ex);
		} catch (Exception ex)
		{
			this.print();
			throw new RuntimeException(ex);
		}
		
		return outStream.toByteArray();
	}
	
	public void print()
	{

		System.out.println("Code Block " + methodImplementation.getMethodDefinition());
		
		System.out.println("Local variables");
		for (LocalVariable localVariable : localVariables)
			System.out.println("\t" + localVariable);		

		System.out.println("Code");
		for (InstructionHandle handle : instructions.getInstructionHandles())
			System.out.println("\t" + handle);		
	}

}
