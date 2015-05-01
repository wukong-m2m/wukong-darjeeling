/*
 * Flags.java
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
 
package org.csiro.darjeeling.infuser.structure;

import java.util.ArrayList;

import org.apache.bcel.classfile.AccessFlags;

/**
 * Flags object, similar to the AccessFlags object in BCEL. 
 * 
 * @author Niels Brouwers
 *
 */
public class Flags
{

	private ArrayList<Flag> flags;
	
	public Flags()
	{
		flags = new ArrayList<Flag>();
	}
	
	/**
	 * Constructs a new Flags object by copying the contents from an AccessFlags object. 
	 * @param accessFlags the AccessFlags object to copy the flags from
	 * @return a new Flags object with the same flags as the AccessFlags object 
	 */
	public static Flags fromAcessFlags(AccessFlags accessFlags)
	{
		Flags ret = new Flags();
		
		if (accessFlags.isPrivate()) ret.flags.add(Flag.PRIVATE); 
		if (accessFlags.isPublic()) ret.flags.add(Flag.PUBLIC); 
		if (accessFlags.isProtected()) ret.flags.add(Flag.PROTECTED); 
		if (accessFlags.isAbstract()) ret.flags.add(Flag.ABSTRACT); 
		if (accessFlags.isFinal()) ret.flags.add(Flag.FINAL); 
		if (accessFlags.isInterface()) ret.flags.add(Flag.INTERFACE); 
		if (accessFlags.isNative()) ret.flags.add(Flag.NATIVE);
		if (accessFlags.isStatic()) ret.flags.add(Flag.STATIC);
		
		return ret;
	}
	
	public boolean contains(Flag flag)
	{
		return flags.contains(flag);
	}
	
	public String toString()
	{
		String ret = "";
		
		for (Flag flag : flags)
			ret += flag.getName() + " ";
		
		return ret.trim();
	}

}
