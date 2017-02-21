/*
 * DIWriterVisitor.java
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
 
package org.csiro.darjeeling.infuser.outputphase;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;

import org.csiro.darjeeling.infuser.bytecode.CodeBlock;
import org.csiro.darjeeling.infuser.bytecode.ExceptionHandler;
import org.csiro.darjeeling.infuser.structure.DescendingVisitor;
import org.csiro.darjeeling.infuser.structure.Element;
import org.csiro.darjeeling.infuser.structure.LocalId;
import org.csiro.darjeeling.infuser.structure.ParentElement;
import org.csiro.darjeeling.infuser.structure.elements.AbstractClassDefinition;
import org.csiro.darjeeling.infuser.structure.elements.AbstractHeader;
import org.csiro.darjeeling.infuser.structure.elements.AbstractInfusion;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethod;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodDefinition;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodDefinitionList;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodImplementation;
import org.csiro.darjeeling.infuser.structure.elements.AbstractReferencedInfusionList;
import org.csiro.darjeeling.infuser.structure.elements.AbstractStaticFieldList;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalClassDefinition;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodImplementation;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalStringTable;

/**
 * The DI Writer Visitor walks over the Infusion element tree and writes out a .di file. Each element is first written
 * into a byte buffer, are then recursively stitched together.  
 * 
 * @author Niels Brouwers
 */
public class DIWriterVisitor extends DescendingVisitor
{

	private BinaryOutputStream out;
	private AbstractInfusion rootInfusion;
	
	public DIWriterVisitor(OutputStream out, AbstractInfusion rootInfusion)
	{
		this.rootInfusion = rootInfusion;
		this.out = new BinaryOutputStream(out);
	}
	

	@Override
	public void visit(AbstractInfusion element)
	{
		try {
			writeChildren(element, 0);			
		} catch (IOException ex)
		{
			throw new RuntimeException(ex);
		}
	}
	
	public void visit(AbstractReferencedInfusionList element)
	{
		try {
			
			// write element id
			out.writeUINT8(element.getId().getId());

			int nrElements = element.getChildren().size();
			
			// write number of elements in list
			out.writeUINT8(nrElements);
			
			// write out forward pointers
			int offset = nrElements * 2 + 1;
			for (int i=0; i<nrElements; i++)
			{
				out.writeUINT16(offset);
				offset += element.get(i).getHeader().getInfusionName().length() + 1;
			}
			
			// write infusion names
			for (int i=0; i<nrElements; i++)
			{
				out.write(element.get(i).getHeader().getInfusionName().getBytes());
				out.write(0);
			}
			
		} catch (IOException ex)
		{
			throw new RuntimeException(ex);
		}

	}

	@Override
	public void visit(AbstractHeader element)
	{
		try {
			out.writeUINT8(element.getId().getId());
			out.writeUINT8(element.getInfusionFormatVersion());
			out.writeUINT8(element.getInfusionVersion());
			
			AbstractMethodImplementation entryPoint = element.getEntryPoint();
			if (entryPoint!=null)
				out.writeUINT8(entryPoint.getGlobalId().getEntityId());
			else
				out.writeUINT8(255);
		
			out.write(element.getInfusionName().getBytes());
			out.write(0);

		} catch (IOException ex)
		{
			throw new RuntimeException(ex);
		}
	}
		
	@Override
	public void visit(InternalClassDefinition element)
	{
		try {
			
			out.writeUINT8(element.getReferenceFieldCount());
			out.writeUINT8(element.getNonReferenceFieldsSize());
			
			LocalId superClassId = new LocalId(0,0);
			if (element.getSuperClass()!=null)
			{
				superClassId = element.getSuperClass().getGlobalId().resolve(rootInfusion);
			} else
			{
				superClassId = new LocalId(255,0);		// darjeeling.Object
			}
			writeLocalId(out, superClassId);
			
			// write <CLINIT> method id
			if (element.getCInit()!=null)
				out.writeUINT8(element.getCInit().getGlobalId().getEntityId());
			else
				out.writeUINT8(255);
			
			// Write name ID
			writeLocalId(out, element.getNameId().resolve(rootInfusion));

			// write interface list
			out.writeUINT8(element.getInterfaces().size());
			for (AbstractClassDefinition classDef : element.getInterfaces())
				writeLocalId(out, classDef.getGlobalId().resolve(rootInfusion));
			
			// write method table
			out.writeUINT8(element.getChildren().size());
			for (AbstractMethod method : element.getChildren())
			{
				writeLocalId(out, method.getMethodDef().getGlobalId().resolve(rootInfusion));
				writeLocalId(out, method.getMethodImpl().getGlobalId().resolve(rootInfusion));
			}
			
		} catch (IOException ex)
		{
			throw new RuntimeException(ex);
		}
	}
	
	@Override
	public void visit(InternalMethodImplementation element)
	{
		
		try {
			
			// write method details
			out.writeUINT8(element.getReferenceArgumentCount());
			out.writeUINT8(element.getIntegerArgumentCount());
			// RTC: need to change the stackframe to include parameters, in order to avoid
			// having to check on each LOAD if it is a parameter or local variable.
			// Originally the parameters were only kept on the stack in the caller's frame,
			// but this requires a check on the index at xLOAD and xSTORE instructions,
			// which is ok in the interpreter, but unacceptably slow for RTC.
			// out.writeUINT8(element.getReferenceLocalVariableCount() - element.getReferenceArgumentCount() - (element.isStatic()?0:1));
			// out.writeUINT8(element.getIntegerLocalVariableCount() - element.getIntegerArgumentCount());
			// Instead we now copy them to the callee's frame's local variable arrays.
			// This wastes some space, and adds overhead to the function start, but means we
			// can index all locals directly.
			out.writeUINT8(element.getReferenceLocalVariableCount());
			out.writeUINT8(element.getIntegerLocalVariableCount());

			// total number of parameters (including 'this')
			out.writeUINT8(element.getMethodDefinition().getParameterCount() + (element.isStatic()?0:1));
			
			out.writeUINT8(element.getMaxStack());
			out.writeUINT8(element.getMaxRefStack());

			CodeBlock code = element.getCodeBlock();

			// Write flags
			int flags = 0;
			if (element.isNative()) flags |= 1;
			if (element.isStatic()) flags |= 2;
			out.writeUINT8(flags);
			
			// Write return type
			out.writeUINT8(element.getMethodDefinition().getReturnType().getTType());

			// Write the number of branch targets
			out.writeUINT16(element.getNumberOfBranchTargets());
			
			// RTC: write offsets to integer variables
			// (storing the offset to reference variable doesn't seem worth it at this point, since it starts
			//  at element.getMaxStack() * sizeof(int16_t). So storing it here only saves one shift left, which
			//  should only take a single cycle)
			// See notes.txt for how dj_frame_getLocalIntegerVariables used to be calculated:
													// dj_frame_getLocalIntegerVariables = (char*)frame + sizeof(dj_frame)
													// + (sizeof(int16_t) * dj_di_methodImplementation_getMaxStack(methodImpl)) \
													// + (sizeof(ref_t) * dj_di_methodImplementation_getReferenceLocalVariableCount(methodImpl)) \
													// + (sizeof(int16_t) * (dj_di_methodImplementation_getIntegerLocalVariableCount(methodImpl)-1))
			// (note the header now assumes 2 byte pointers, so VMs on larger architectures will need to do some extra work!)
			out.writeUINT8(element.getReferenceLocalVariableCount() + element.getIntegerLocalVariableCount());

			// write code block
			if (element.getCode()==null)
			{
				out.writeUINT16(0);
			} else
			{
				// write code length in bytes
				byte[] codeArray = code.toByteArray();
				out.writeUINT16(codeArray.length);

				// write byte code
				out.write(codeArray);
			}
			
			// write exception table
			if (code!=null)
			{
				ExceptionHandler exceptions[] = code.getExceptionHandlers();
				out.writeUINT8(exceptions.length);
				for (ExceptionHandler exception : exceptions)
				{
					out.writeUINT8(exception.getCatchType().getInfusionId());
					out.writeUINT8(exception.getCatchType().getLocalId());
					out.writeUINT16(exception.getStart().getPc());
					out.writeUINT16(exception.getEnd().getPc());
					out.writeUINT16(exception.getHandler().getPc());
				}
				
			} else
			{
				out.writeUINT8(0);
			}
			
		} catch (IOException ex)
		{
			throw new RuntimeException(ex);
		}
	}
	

	private void writeLocalId(BinaryOutputStream out, LocalId id) throws IOException
	{
		out.writeUINT8(id.getInfusionId());
		out.writeUINT8(id.getLocalId());
	}
	
	@Override
	public void visit(AbstractStaticFieldList element)
	{
		try {
			// element id
			out.writeUINT8(element.getId().getId());
			
			// field count
			out.writeUINT8(element.getNrRefs());
			out.writeUINT8(element.getNrBytes());
			out.writeUINT8(element.getNrShorts());
			out.writeUINT8(element.getNrInts());
			out.writeUINT8(element.getNrLongs());
		} catch (IOException ex)
		{
			throw new RuntimeException(ex);
		}
	}
	
	@Override
	public void visit(InternalStringTable element)
	{
		
		try {
			// element id
			out.writeUINT8(element.getId().getId());
			
			// string count
			String[] strings = element.elements();
			out.writeUINT16(strings.length);
			
			// forward table (from beginning of element)
			int offset = strings.length*2 + 3;
			for (int i=0; i<strings.length; i++)
			{
				out.writeUINT16(offset);
				offset+=strings[i].length()+2;
			}
			
			// strings (excluding trailing 0!)
			for (int i=0; i<strings.length; i++)
			{
				// string length
				out.writeUINT16(strings[i].length());
				
				// string bytes
				out.write(strings[i].getBytes());
			}
			
		} catch (IOException ex)
		{
			throw new RuntimeException(ex);
		}
	}
	
	@Override
	public <T extends Element> void visit(ParentElement<T> element)
	{
		try {
			// element id
			out.writeUINT8(element.getId().getId());
			
			// write child elements
			writeChildren(element, 1);
		} catch (IOException ex)
		{
			throw new RuntimeException(ex);
		}
	}
	
	private <T extends Element> void writeChildren(ParentElement<T> element, int offset) throws IOException
	{
		
		// TODO refactor this legacy crap
		ArrayList<T> exportChildren = new ArrayList<T>();  
		for (T child : element.getChildren()) exportChildren.add(child);
		
		// preconditions
		// TODO: move this to the pre-check step
		assert(exportChildren.size()<256) : "Number of children in parent element must not exceed 255";

		// write number of elements in list
		out.writeUINT8(exportChildren.size());

		// serialise all the list elements to byte arrays
		byte[][] serializedElements = new byte[exportChildren.size()][];
		int i = 0;
		for (T child : exportChildren)
		{
			ByteArrayOutputStream outStream = new ByteArrayOutputStream();
			DIWriterVisitor visitor = new DIWriterVisitor(outStream, rootInfusion);
			child.accept(visitor);
			serializedElements[i] = outStream.toByteArray();
			outStream.close();
			i++;
		}
		
		// write forward pointer table.
		// pointer to the first element is at is 2 bytes (list size is a u16), 
		// plus the size of the table itself
		int pointer = offset + 1 + 2*(exportChildren.size());
		for (i=0; i<exportChildren.size(); i++)
		{
			out.writeUINT16(pointer);
			pointer += serializedElements[i].length;
		}
		
		// write the serialized list elements
		for (i=0; i<exportChildren.size(); i++)
			out.write(serializedElements[i]);			
		
	}
	
	@Override
	public void visit(AbstractMethodDefinitionList element)
	{
		try {
			// element id
			out.writeUINT8(element.getId().getId());
			
			// write number of method definitions
			out.writeUINT8(element.getChildren().size());
			
			// write nr_args for each method definition
			for (AbstractMethodDefinition methodDef : element.getChildren())
			{
				// methodDef.getDescriptor()
				int nrArgs = methodDef.getParameterCount();
				out.writeUINT8(nrArgs);
			}
			
		} catch (IOException ex)
		{
			throw new RuntimeException(ex);
		}
	}
	
	@Override
	public void visit(Element element)
	{
		try {
			// element id
			out.writeUINT8(element.getId().getId());
		} catch (IOException ex)
		{
			throw new RuntimeException(ex);
		}
	}	
	
}
