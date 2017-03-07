/*
 * Buttons.java
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
 * API for working with buttons.  
 * 
 * @author Niels Brouwers
 *
 */
public class Buttons
{
	
	/**
	 * @return number of available buttons. 
	 */
	public static native short getNrButtons();
	
	/**
	 * Checks if a button is pressed. 
	 * @param nr button number.
	 * @return true iff the button is pressed.
	 */
	public static native boolean pressed(short nr);

	/**
	 * Suspends the thread until a specific button is pressed (down). 
	 * @param nr button to wait for.
	 */
	public static native void waitForDown(short nr);
	
	/**
	 * Suspends the thread until a specific button is raised (down). 
	 * @param nr button to wait for.
	 */
	public static native void waitForUp(short nr);

}
