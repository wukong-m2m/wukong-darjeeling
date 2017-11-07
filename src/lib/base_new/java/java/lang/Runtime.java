/*
 * Runtime.java
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
 
package java.lang;


public class Runtime 
{

	private static Runtime currentRuntime = new Runtime();

	/**
	 * Returns the runtime object associated with the current Java application.
	 * Most of the methods of class <code>Runtime</code> are instance methods
	 * and must be invoked with respect to the current runtime object.
	 * 
	 * @return the <code>Runtime</code> object associated with the current Java
	 *         application.
	 */
	public static Runtime getRuntime() {
		return currentRuntime;
	}

	/** Don't let anyone else instantiate this class */
	private Runtime() {
	}

	/**
	 * Terminates the currently running Java application. This method never
	 * returns normally.
	 * <p>
	 * The argument serves as a status code; by convention, a nonzero status
	 * code indicates abnormal termination.
	 * 
	 * @param status
	 *            exit status.
	 * @since JDK1.0
	 */
	public native void exit(int status);

	/**
	 * Returns the amount of free memory in the system. Calling the
	 * <code>gc</code> method may result in increasing the value returned by
	 * <code>freeMemory.</code>
	 * 
	 * @return an approximation to the total amount of memory currently
	 *         available for future allocated objects, measured in bytes.
	 */
	public native int freeMemory();

	/**
	 * Returns the total amount of memory in the Java Virtual Machine. The value
	 * returned by this method may vary over time, depending on the host
	 * environment.
	 * <p>
	 * Note that the amount of memory required to hold an object of any given
	 * type may be implementation-dependent.
	 * 
	 * @return the total amount of memory currently available for current and
	 *         future objects, measured in bytes.
	 */
	public native int totalMemory();

	/**
	 * Runs the garbage collector. Calling this method suggests that the Java
	 * Virtual Machine expend effort toward recycling unused objects in order to
	 * make the memory they currently occupy available for quick reuse. When
	 * control returns from the method call, the Java Virtual Machine has made
	 * its best effort to recycle all discarded objects.
	 * <p>
	 * The name <code>gc</code> stands for "garbage collector". The Java Virtual
	 * Machine performs this recycling process automatically as needed, in a
	 * separate thread, even if the <code>gc</code> method is not invoked
	 * explicitly.
	 * <p>
	 * The method {@link System#gc()} is hte conventional and convenient means
	 * of invoking this method.
	 */
	public native void gc();

}
