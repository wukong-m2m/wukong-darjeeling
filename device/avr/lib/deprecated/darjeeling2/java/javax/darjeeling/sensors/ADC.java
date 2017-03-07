/*
 * ADC.java
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
 
 
package javax.darjeeling.sensors;

/**
 * 
 * API for ADC (analog-to-digital) converters.
 * 
 * @author Niels Brouwers
 *
 */
public class ADC
{

	/**
	 * @return number of available ADC channels.
	 */
	public static native short getNrADCs();
	
	/**
	 * Reads a value from the an ADC channel.
	 * @param channel ADC channel. 
	 * @return measured value.
	 */
	public static native int read(short channel);

	/**
	 * Gets the resolution for a given ADC channel.
	 * @param channel ADC channel number.
	 * @return resolution in bits.
	 */
	public static native byte getResolution(short channel);
	

}
