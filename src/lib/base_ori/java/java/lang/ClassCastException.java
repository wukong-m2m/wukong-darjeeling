/*
 * Copyright (c) 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * 
 */
package java.lang;

/**
 * Thrown when an application attempts to cast a reference to an Object of
 * inappropriate type.
 */
public class ClassCastException extends RuntimeException
{

	/**
	 * Constructs a <code>ClassCastException</code> with no detail message.
	 */
	public ClassCastException()
	{
		super();
	}

	/**
	 * Constructs a <code>ClassCastException</code> with the specified detail
	 * message.
	 * 
	 * @param s
	 *            the detail message.
	 */
	public ClassCastException(String s)
	{
		super(s);
	}
}
