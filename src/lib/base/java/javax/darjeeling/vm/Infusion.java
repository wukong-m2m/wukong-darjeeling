/*
 * Infusion.java
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
 
package javax.darjeeling.vm;


public class Infusion
{
	
	private Object internalInfusion;
	
	private Infusion(Object internalInfusion)
	{
		this.internalInfusion = internalInfusion;
	}
	
	public static native short getInfusionCount();
	
	private static native Object _getInfusion(short index);
	
	public static Infusion getInfusion(short index)
	{
		// check range
		if (index<0||index>getInfusionCount())
			throw new IndexOutOfBoundsException();
		
		// create new instance
		return new Infusion(_getInfusion(index));
	}
	
	private static native char[] _getName(Object internalInfusion);	
	
	public String getName()
	{
		return new String(_getName(internalInfusion));		
	}
	
	private static native short _getImportedInfusionCount(Object internalInfusion);
	
	public short getImportedInfusionCount()
	{
		return _getImportedInfusionCount(internalInfusion);
	}
	
	private static native Object _getImportedInfusion(Object internalInfusion, short index);
	
	public Infusion getImportedInfusion(short index)
	{
		// check range
		if (index<0||index>_getImportedInfusionCount(internalInfusion))
			throw new IndexOutOfBoundsException();
		
		return new Infusion(_getImportedInfusion(internalInfusion, index));
	}
	
	public static Infusion getInfusionByName(String name)
	{
		for (short i=0; i<getInfusionCount(); i++)
		{
			Object internalInfusion = _getInfusion(i);
			if (new String(_getName(internalInfusion)).equals(name))
				return new Infusion(internalInfusion);
		}		
		
		return null;
	}
	
	private native void _unload(Object internalInfusion) throws InfusionUnloadDependencyException;
	
	public void unload() throws InfusionUnloadDependencyException
	{
		_unload(internalInfusion);
	}

}
