/*
 * Message.java
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

public class Message
{
	
	public enum Type
	{
		ERROR,
		WARNING
	}
	
	private final Type type;
	private final String file;
	private final int lineNumber;
	private final String message;
	
	public Message(Type type, String file, int lineNumber, String message)
	{
		this.type = type;
		this.file = file;
		this.lineNumber = lineNumber;
		this.message = message;
	}

	public String toString()
	{
		String ret = "";
		
		if (file!=null) ret += file + ":";
		if (lineNumber!=-1) ret += lineNumber + ":";
		ret += (type==Type.ERROR)?" error: ":" warning: ";
		ret += message;
		
		return ret; 
	}
	
	public Type getType()
	{
		return type;
	}
	
}
