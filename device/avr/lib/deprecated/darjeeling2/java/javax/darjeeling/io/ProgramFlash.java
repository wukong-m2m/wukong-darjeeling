/*
 * ProgramFlash.java
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
 
 
package javax.darjeeling.io;

/**
 * The ProgramFlash class is a singleton that provides access to the program flash of the MCU where the Darjeeling infusion files are
 * stored.
 * 
 * TODO WORK IN PROGRESS
 * 
 * @author Niels Brouwers
 *
 */
public class ProgramFlash extends BlockDevice
{

	// Singleton instance
	private static ProgramFlash instance;

	// Native interface
	public native short getBlockSize();
	public native int getBlockCount();
	public native byte[] _read(int blockIndex);
	public native void _write(int blockIndex, byte[] data);
	
	// Constructor private - singleton.
	private ProgramFlash()
	{
	}
	
	public void write(int blockIndex, byte[] data)
	{
		// Check preconditions.
		if (blockIndex<0 || blockIndex>=getBlockCount()) throw new IndexOutOfBoundsException();
		if (data==null) throw new NullPointerException();
		if (data.length!=getBlockSize()) throw new IllegalArgumentException();
		
		_write(blockIndex, data);
	}
	
	public byte[] read(int blockIndex)
	{
		// Check preconditions.
		if (blockIndex<0 || blockIndex>=getBlockCount()) throw new IndexOutOfBoundsException();
		
		return _read(blockIndex);
	}
	
	/**
	 * @return the ProgramFlash instance. 
	 */
	public static ProgramFlash getInstance()
	{
		// TODO: not thread safe
		if (instance==null)
			instance = new ProgramFlash();
		
		return instance;
	}
	

}
