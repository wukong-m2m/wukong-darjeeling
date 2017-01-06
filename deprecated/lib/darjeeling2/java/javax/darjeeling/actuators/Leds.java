/*
 * Leds.java
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
 * The Leds class provides an API to drive any leds on the device. 
 * 
 * @author Niels Brouwers
 *
 */
public class Leds
{
	
	/**
	 * Returns the number of available leds on the device. 
	 * @return number of available leds
	 */
	public static native short getNrLeds();
	
	/**
	 * Sets the state of a specific led. 
	 * @param nr led index. Should be in the range [0, nr_leds>. 
	 * @param status status of the led. True means the led is to be turned on, false means off.
	 */
	public static native void set(short nr, boolean status);

}
