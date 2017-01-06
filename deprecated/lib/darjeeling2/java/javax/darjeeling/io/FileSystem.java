/*
 * FileSystem.java
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
 * The abstract FileSystem class represents a file sytem on top of a block device. 
 * 
 * TODO WORK IN PROGRESS
 *
 * @author Niels Brouwers
 *
 */
public abstract class FileSystem
{
	
	protected BlockDevice blockDevice;
	
	/**
	 * Constructs a new FileSystem instance.
	 * 
	 * @param blockDevice block device. 
	 */
	public FileSystem(BlockDevice blockDevice)
	{
		this.blockDevice = blockDevice;
	}
	
	public abstract void mount();
	public abstract void unmount();

}
