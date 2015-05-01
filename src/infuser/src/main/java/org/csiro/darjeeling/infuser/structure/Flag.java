/*
 * Flag.java
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

public enum Flag
{
	
	PUBLIC("public"),
	PRIVATE("private"),
	PROTECTED("protected"),
	ABSTRACT("abstract"),
	FINAL("final"),
	INTERFACE("interface"),
	NATIVE("native"),
	STATIC("static");
	
	String name;
	
	private Flag(String name)
	{
		this.name = name;
	}
	
	public long getMask()
	{
		return 1 << this.ordinal();
	}
	
	public String getName()
	{
		return name;
	}

}
