/*
 * SerialOutputStream.java
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

import java.io.OutputStream;

/**
 * OutputStream implementation for serial ports (UART).
 * 
 * @author Niels Brouwers
 *
 */
public class SerialOutputStream extends OutputStream
{

	// Serial port number.
	private byte uartNr;

	private static native void _write(byte uartNr, int value);
	
	protected SerialOutputStream(byte uartNr)
	{
		this.uartNr = uartNr;
	}

	public void write(int b)
	{
		_write(uartNr, b);
	}
	

}
