/*
 * BinaryOutputStream.java
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

import java.io.IOException;
import java.io.OutputStream;

public class BinaryOutputStream extends OutputStream
{

	private OutputStream outStream;
	
	public BinaryOutputStream(OutputStream outStream)
	{
		this.outStream = outStream;
	}
	
	@Override
	public void write(int b) throws IOException
	{
		outStream.write(b);
	} 
	
	public void writeUINT8(int value) throws IOException
	{
		outStream.write(value & 255);
	}

	public void writeUINT16(int value) throws IOException
	{
		outStream.write(value & 255);
		outStream.write((value>>8) & 255);
	}

	public void writeUINT32(long value) throws IOException
	{
		outStream.write((int)(value & 255));
		outStream.write((int)((value>>8) & 255));
		outStream.write((int)((value>>16) & 255));
		outStream.write((int)((value>>24) & 255));
	}

}
