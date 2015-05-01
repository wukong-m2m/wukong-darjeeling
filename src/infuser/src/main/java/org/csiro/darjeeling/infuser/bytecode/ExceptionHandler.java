/*
 * ExceptionHandler.java
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

import org.csiro.darjeeling.infuser.structure.LocalId;

/**
 * 
 * Exception handler (try/catch) block.     
 * 
 * @author Niels Brouwers
 *
 */
public class ExceptionHandler
{
	
	// the range this handler is active in (try { ... })
	private InstructionHandle start, end;
	
	// the handler (catch { ... })
	private InstructionHandle handler;
	
	// the Throwable type this block handles 
	private LocalId catchType;
	
	/**
	 * Creates a new ExceptionHandler instance.
	 * @param start the first instruction of the try block
	 * @param end the last instruction of the try block
	 * @param handler the first instruction of the catch block
	 * @param catchType the type of Throwable handled by this exception handler 
	 */
	public ExceptionHandler(InstructionHandle start, InstructionHandle end, InstructionHandle handler, LocalId catchType)
	{
		this.start = start;
		this.end = end;
		this.handler = handler;
		this.catchType = catchType;
	}
	
	/**
	 * @return the first instruction of the try block
	 */
	public InstructionHandle getStart()
	{
		return start;
	}
	
	/**
	 * @return the last instruction of the try block
	 */
	public InstructionHandle getEnd()
	{
		return end;
	}
	
	/**
	 * @return the first instruction of the catch block
	 */
	public InstructionHandle getHandler()
	{
		return handler;
	}
	
	/**
	 * @return the type of Throwable handled by this handler
	 */
	public LocalId getCatchType()
	{
		return catchType;
	}

}
