/*
 *   
 *
 * Copyright  1990-2007 Sun Microsystems, Inc. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 only, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details (a copy is
 * included at /legal/license.txt).
 * 
 * You should have received a copy of the GNU General Public License
 * version 2 along with this work; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 * 
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa
 * Clara, CA 95054 or visit www.sun.com if you need additional
 * information or have any questions.
 */

package java.lang;

/**
 * The Character class wraps a value of the primitive type <code>char</code> in
 * an object. An object of type <code>Character</code> contains a single field
 * whose type is <code>char</code>.
 * <p>
 * In addition, this class provides several methods for determining the type of
 * a character and converting characters from uppercase to lowercase and vice
 * versa.
 * <p>
 * Character information is based on the Unicode Standard, version 3.0. However,
 * in order to reduce footprint, by default the character property and case
 * conversion operations in CLDC are available only for the ISO Latin-1 range of
 * characters. Other Unicode character blocks can be supported as necessary.
 * <p>
 * 
 * @version 12/17/01 (CLDC 1.1)
 * @since JDK1.0, CLDC 1.0
 */
public final class Character extends Object
{
	/**
	 * The value of the Character.
	 */
	private char value;

	/**
	 * Constructs a <code>Character</code> object and initializes it so that it
	 * represents the primitive <code>value</code> argument.
	 * 
	 * @param value
	 *            value for the new <code>Character</code> object.
	 */
	public Character(char value)
	{
		this.value = value;
	}

	/**
	 * Returns the value of this Character object.
	 * 
	 * @return the primitive <code>char</code> value represented by this object.
	 */
	public char charValue()
	{
		return value;
	}

	/**
	 * Returns a String object representing this character's value. Converts
	 * this <code>Character</code> object to a string. The result is a string
	 * whose length is <code>1</code>. The string's sole component is the
	 * primitive <code>char</code> value represented by this object.
	 * 
	 * @return a string representation of this object.
	 */
	public String toString()
	{
		// char buf[] = {value};
		// return String.valueOf(buf);
		return null;
	}
}
