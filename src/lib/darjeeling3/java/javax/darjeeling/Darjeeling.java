/*
 * Darjeeling.java
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
 
package javax.darjeeling;

import java.io.PrintStream;

/**
 * 
 * The Darjeeling class contains some general purpose methods such as printing to the default console, an assertion method for 
 * unit testing, and some other bits and pieces.
 * 
 * @author Niels Brouwers
 *
 */
public class Darjeeling
{
	static {
		System.out = new PrintStream(new ConsoleOutputStream());
	}
	
	/**
	 * Assertion method for unit testing purposes. Prints the test nr and wether the test failed or passed.
	 * @param testNr test nr.
	 * @param success whether the test failed or passed, true being a pass.
	 */
	public static native void assertTrue(int testNr, boolean success);

	private static native void _print(String string);

	/**
	 * Prints a String to the default console. 
	 * @param str string to print
	 */
	public static void print(String str)
	{
		if (str==null) throw new NullPointerException();
		_print(str);
	}

	/**
	 * @return the amount of free memory, in bytes.
	 */
	public static native int getMemFree();
	
	/**
	 * @return an integer that represents a unique node ID. 
	 */
	public static native int getNodeId();
	
	/**
	 * @return a random integer value.
	 */
	public static native int random();
	
	/**
	 * @return number of threads in the VM.
	 */
	public static native short getNrThreads();
	
	/**
	 * Gets a thread by it's index in the thread list. Can be used together with getNrThreads to iterate through all threads. 
	 * @param nr thread nr
	 * @return thread at index [nr].
	 */
	public static native Thread getThread(short nr);
}
