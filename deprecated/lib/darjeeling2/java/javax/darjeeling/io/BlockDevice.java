/*
 * BlockDevice.java
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
 * 
 * The BlockDevice class allows access to writable flash commonly found on sensor nodes. These include data storage facilities such as 
 * on-board flash or an SD slot, and the program flash of an MCU. The latter can be used for in situ reprogramming. 
 * 
 * @author Niels Brouwers
 *
 */
public abstract class BlockDevice
{
	
	/**
	 * @return block size in bytes.
	 */
	public abstract short getBlockSize();
	
	/**
	 * @return number of blocks;
	 */
	public abstract int getBlockCount();
	
	/**
	 * @return the size of the device in bytes.
	 */
	public int getSizeInBytes()
	{
		return getBlockCount() * getBlockSize();
	}
	
	/**
	 * Reads a single block from the device and returns it as a byte array. The blocks are indexed by block number, so for a block size of 
	 * n, the first block is at offset 0, the second at n, the third at 2*n, etc.
	 * 
	 * Throws an <code>OutOfBoundsException</code> is blockIndex is outside [0, <code>getBlockCount()</code>. 
	 * 
	 * @param index block index.  
	 * @return a byte array containing the contents of the requested block.
	 */
	public abstract byte[] read(int blockIndex);

	/**
	 * Writes a block of data to the device. The data byte array should be of size <code>getBlockSize()</code>, if not, an 
	 * <code>IllegalArgumentException</code> is thrown.
	 * 
	 * Throws an <code>OutOfBoundsException</code> is blockIndex is outside [0, <code>getBlockCount()</code>. 
	 *  
	 * @param blockIndex
	 * @param data
	 */
	public abstract void write(int blockIndex, byte[] data);

}
