/*
 * Serial.java
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
 * The Serial class provides access to the serial ports of the device. 
 * 
 * @author Niels Brouwers
 *
 */
public class Serial
{
	
	/**
	 * Sets the desired baud rate of a serial port. 
	 * 
	 * @param uartNr serial port number. Must be a valid number. Use getNrSerialPorts() to query the number of available uarts. Throws an IndexOutOfRangeException if the serial port does not exist.
	 * @param baudRate desired baud rate. 
	 */
	// TODO figure out what should happen if the baud rate is not available
	public static native void setBaudRate(byte uartNr, int baudRate);
	
	/**
	 * @return number of available serial ports.
	 */
	public static native byte getNrSerialPorts();
	
	/**
	 * @param uartNr uartNr serial port number. Must be a valid number. Use getNrSerialPorts() to query the number of available uarts. Throws an IndexOutOfRangeException if the serial port does not exist. 
	 * @return default baud rate.
	 */
	public static native int getDefaultBaudRate(byte uartNr);
	
	/**
	 * Gets an input stream for the given serial port. 
	 * @param uartNr uartNr serial port number. Must be a valid number. Use getNrSerialPorts() to query the number of available uarts. Throws an IndexOutOfRangeException if the serial port does not exist. 
	 * @return a SerialInputStream object that is connected to the given uart.
	 */
	public static SerialInputStream getInputStream(byte uartNr)
	{
		// Check that the uart number is in range.
		if (uartNr<0 || uartNr >= getNrSerialPorts())
			throw new IndexOutOfBoundsException();
		
		return new SerialInputStream(uartNr);
	}
	
	/**
	 * Gets an output stream for the given serial port. 
	 * @param uartNr uartNr serial port number. Must be a valid number. Use getNrSerialPorts() to query the number of available uarts. Throws an IndexOutOfRangeException if the serial port does not exist. 
	 * @return a SerialOutputStream object that is connected to the given uart.
	 */
	public static SerialOutputStream getOutputStream(byte uartNr)
	{
		// Check that the uart number is in range.
		if (uartNr<0 || uartNr >= getNrSerialPorts())
			throw new IndexOutOfBoundsException();
		
		return new SerialOutputStream(uartNr);
	}
	

}
