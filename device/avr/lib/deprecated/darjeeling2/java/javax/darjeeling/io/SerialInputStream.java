/*
 * SerialInputStream.java
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

import java.io.InputStream;

/**
 * InputStream implementation for serial ports (UART).
 * 
 * @author Niels Brouwers
 *
 */
public class SerialInputStream extends InputStream
{
	
	// Serial port number.
	private byte uartNr;
	
	private static native void _waitForByte(byte uartNr);
	private static native short _read(byte uartNr);
	
	/**
	 * Creates a new SerialInputStream given a serial port number.
	 * 
	 * @param uartNr
	 * @param baudRate
	 * @throws SerialPortInUseException
	 */
	public SerialInputStream(byte uartNr)
	{
		this.uartNr = uartNr;
	}
	
	public int read()
	{
		synchronized(this)
		{
			_waitForByte(uartNr);
			return _read(uartNr);
		}
	}

}
