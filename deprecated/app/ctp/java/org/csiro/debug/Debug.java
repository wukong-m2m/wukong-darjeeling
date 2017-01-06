/*
 * Debug.java
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
 
package org.csiro.debug;

import javax.darjeeling.Darjeeling;

/**
 * A thin layer on top of Darjeeling.print to make sure no two threads go into
 * print at the same time. This is because we want to avoid race conditions in
 * the tinyOS split control stuff.
 * 
 * @author Niels Brouwers
 * 
 */
public class Debug {

	private static Object lock = new Object();
	private static boolean displayOutput = true;

	public static void setDisplayOutput(boolean on) {
		displayOutput = on;
	}

	public static void print(String str) {
		if (!displayOutput)
			return;

		synchronized (lock) {
			Darjeeling.print(str);
		}
	}

	public static void print(byte[] array) {
		if (!displayOutput)
			return;

		synchronized (lock) {
			Darjeeling.print(array);
		}
	}

	public static void print(int i) {
		if (!displayOutput)
			return;

		synchronized (lock) {
			Darjeeling.print(i);
		}
	}

}
