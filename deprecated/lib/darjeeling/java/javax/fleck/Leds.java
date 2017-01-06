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
 
package javax.fleck ;

/**
 * Controls the leds on the Fleck.
 * 
 * @author Niels Brouwers
 *
 */
public class Leds
{
    public static final byte YELLOW=0;
    public static final byte BLUE=0;
    public static final byte GREEN=1;
    public static final byte RED=2;
    
	/**
	 * Controls a led on the fleck board.
	 * @param led the led to set (0, 1, 2)
	 * @param state the state of the led. (true=on, false=off)
	 */
    public static native void setLed(int led, boolean state);

}
