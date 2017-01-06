/*
 * Servo.java
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
 
package javax.darjeeling.actuators;

/**
 * The Servo class provides an API for working with standard r/c servos. A 20 millisecond interval is assumed, with a 1-2 ms pulse width.
 * 
 * @author Niels Brouwers
 *
 */
public class Servo
{
	
	/**
	 * @return the number of available servo channels.
	 */
	public static native short getNrServos();
	
	/**
	 * Sets the position of a servo. 
	 * @param nr servo number. Should be between 0 and getNrServos(), otherwise an IndexOutOfBoundsException is thrown.
	 * @param value servo position. Should be in the range -32768 (full left) and 32767 (full right). 
	 */
	public static native void set(short nr, short value);

	/**
	 * Sets the pulse width range for a servo. Most servos accept a pulse width between 1 and 2 milliseconds every 20 milliseconds,
	 * where 1 is associated with full left, 2 with full right, and 1.5 with centre. Some servos however accept a wider range, or
	 * have a centre position that is not exactly at 1.5 milliseconds. This function therefore allows for the configuration of the 
	 * servo pulse range. 
	 * 
	 * @param nr servo number. Should be between 0 and getNrServos(), otherwise an IndexOutOfBoundsException is thrown.
	 * @param min minimum pulse width in microseconds. Defaults to 1000.
	 * @param max maximum pulse width in microseconds. Defaults to 2000.
	 */
	public static native void setPulseRange(short nr, short min, short max);

}
