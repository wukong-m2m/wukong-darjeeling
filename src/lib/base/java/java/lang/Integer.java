/*
 * Copyright (c) 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 */
package java.lang;

/**
 * The Integer class wraps a value of the primitive type <code>int</code>
 * in an object. An object of type <code>Integer</code> contains a
 * single field whose type is <code>int</code>.
 * <p>
 * In addition, this class provides several methods for converting
 * an <code>int</code> to a <code>String</code> and a
 * <code>String</code> to an <code>int</code>, as well as other
 * constants and methods useful when dealing with an
 * <code>int</code>.
 *
 * @author  Lee Boynton
 * @author  Arthur van Hoff
 * @version 12/17/01 (CLDC 1.1)
 * @since   JDK1.0, CLDC 1.0
 */
public final class Integer {
    /**
     * All possible chars for representing a number as a String
     */
    final static char[] digits = {
        '0', '1', '2', '3', '4', '5',
        '6', '7', '8', '9', 'a', 'b',
        'c', 'd', 'e', 'f'
        // , 'g', 'h',
        // 'i', 'j', 'k', 'l', 'm', 'n',
        // 'o', 'p', 'q', 'r', 's', 't',
        // 'u', 'v', 'w', 'x', 'y', 'z'
    };

    /**
     * Creates a string representation of the first argument in the
     * radix specified by the second argument.
     * <p>
     * If the radix is smaller than <code>Character.MIN_RADIX</code> or
     * larger than <code>Character.MAX_RADIX</code>, then the radix
     * <code>10</code> is used instead.
     * <p>
     * If the first argument is negative, the first element of the
     * result is the ASCII minus character <code>'-'</code>
     * (<tt>'&#92;u002d'</tt>). If the first
     * argument is not negative, no sign character appears in the result.
     * <p>
     * The remaining characters of the result represent the magnitude of
     * the first argument. If the magnitude is zero, it is represented by
     * a single zero character <tt>'0'</tt> (<tt>'&#92;u0030'</tt>); otherwise,
     * the first character of the representation of the magnitude will
     * not be the zero character.
     * The following ASCII characters are used as digits:
     * <blockquote><pre>
     *   0123456789abcdefghijklmnopqrstuvwxyz
     * </pre></blockquote>
     * These are <tt>'&#92;u0030'</tt> through <tt>'&#92;u0039'</tt> and
     * <tt>'&#92;u0061'</tt> through <tt>'&#92;u007a'</tt>. If the
     * <tt>radix</tt> is <var>N</var>, then the first <var>N</var> of these
     * characters are used as radix-<var>N</var> digits in the order shown.
     * Thus, the digits for hexadecimal (radix 16) are
     * <blockquote><pre>
     * <tt>0123456789abcdef</tt>.
     * </pre></blockquote>
     *
     * @param   i       an integer.
     * @param   radix   the radix.
     * @return  a string representation of the argument in the specified radix.
     * @see     java.lang.Character#MAX_RADIX
     * @see     java.lang.Character#MIN_RADIX
     */
    public static String toString(int i, int radix) {
        // if (radix < Character.MIN_RADIX || radix > Character.MAX_RADIX) {
        if (radix < 2 || radix > 16) {
            radix = 10;
        }
        char buf[] = new char[33];
        boolean negative = (i < 0);
        short charPos = 32;
        if (!negative) {
            i = -i;
        }
        while (i <= -radix) {
            buf[charPos--] = digits[-(i % radix)];
            i = i / radix;
        }
        buf[charPos] = digits[-i];
        if (negative) {
            buf[--charPos] = '-';
        }
        return new String(buf, charPos, (short)(33 - charPos));
    }

    /**
     * Creates a string representation of the integer argument as an
     * unsigned integer in base&nbsp;16.
     * <p>
     * The unsigned integer value is the argument plus 2<sup>32</sup> if
     * the argument is negative; otherwise, it is equal to the argument.
     * This value is converted to a string of ASCII digits in hexadecimal
     * (base&nbsp;16) with no extra leading <code>0</code>s. If the
     * unsigned magnitude is zero, it is represented by a single zero
     * character <tt>'0'</tt> (<tt>'&#92;u0030'</tt>); otherwise, the first
     * character of the representation of the unsigned magnitude will
     * not be the zero character. The following characters are used as
     * hexadecimal digits:
     * <blockquote><pre>
     * 0123456789abcdef
     * </pre></blockquote>
     * These are the characters <tt>'&#92;u0030'</tt> through <tt>'&#92;u0039'</tt>
     * and <tt>'u\0039'</tt> through <tt>'&#92;u0066'</tt>.
     *
     * @param   i   an integer.
     * @return  the string representation of the unsigned integer value
     *          represented by the argument in hexadecimal (base&nbsp;16).
     * @since   JDK1.0.2
     */
    public static String toHexString(int i) {
        return toUnsignedString(i, 4);
    }

    /**
     * Creates a string representation of the integer argument as an
     * unsigned integer in base 8.
     * <p>
     * The unsigned integer value is the argument plus 2<sup>32</sup> if
     * the argument is negative; otherwise, it is equal to the argument.
     * This value is converted to a string of ASCII digits in octal
     * (base&nbsp;8) with no extra leading <code>0</code>s.
     * <p>
     * If the unsigned magnitude is zero, it is represented by a single
     * zero character <tt>'0'</tt> (<tt>'&#92;u0030'</tt>); otherwise, the
     * first character of the representation of the unsigned magnitude will
     * not be the zero character. The octal digits are:
     * <blockquote><pre>
     * 01234567
     * </pre></blockquote>
     * These are the characters <tt>'&#92;u0030'</tt> through <tt>'&#92;u0037'</tt>.
     *
     * @param   i   an integer
     * @return  the string representation of the unsigned integer value
     *          represented by the argument in octal (base&nbsp;8).
     * @since   JDK1.0.2
     */
    public static String toOctalString(int i) {
        return toUnsignedString(i, 3);
    }

    /**
     * Creates a string representation of the integer argument as an
     * unsigned integer in base&nbsp;2.
     * <p>
     * The unsigned integer value is the argument plus 2<sup>32</sup>if
     * the argument is negative; otherwise it is equal to the argument.
     * This value is converted to a string of ASCII digits in binary
     * (base&nbsp;2) with no extra leading <code>0</code>s.
     *
     * If the unsigned magnitude is zero, it is represented by a single
     * zero character <tt>'0'</tt> (<tt>'&#92;u0030'</tt>); otherwise, the
     * first character of the representation of the unsigned magnitude
     * will not be the zero character. The characters <tt>'0'</tt>
     * (<tt>'&#92;u0030'</tt>) and <tt>'1'</tt> (<tt>'&#92;u0031'</tt>) are used
     * as binary digits.
     *
     * @param   i   an integer.
     * @return  the string representation of the unsigned integer value
     *          represented by the argument in binary (base&nbsp;2).
     * @since   JDK1.0.2
     */
    public static String toBinaryString(int i) {
        return toUnsignedString(i, 1);
    }

    /**
     * Convert the integer to an unsigned number.
     */
    private static String toUnsignedString(int i, int shift) {
        char[] buf = new char[32];
        short charPos = 32;
        int radix = 1 << shift;
        int mask = radix - 1;
        do {
            buf[--charPos] = digits[i & mask];
            i >>>= shift;
        } while (i != 0);
        return new String(buf, charPos, (short)(32 - charPos));
    }

    /**
     * Returns a new String object representing the specified integer. The
     * argument is converted to signed decimal representation and returned
     * as a string, exactly as if the argument and radix <tt>10</tt> were
     * given as arguments to the {@link #toString(int, int)} method.
     *
     * @param   i   an integer to be converted.
     * @return  a string representation of the argument in base&nbsp;10.
     */
    public static String toString(int i) {
        return toString(i, 10);
    }

    /**
     * The value of the Integer.
     *
     * @serial
     */
    private int value;

    /**
     * Constructs a newly allocated <code>Integer</code> object that
     * represents the primitive <code>int</code> argument.
     *
     * @param   value   the value to be represented by the <code>Integer</code>.
     */
    public Integer(int value) {
        this.value = value;
    }

    /**
     * Returns the value of this Integer as an int.
     *
     * @return  the <code>int</code> value represented by this object.
     */
    public int intValue() {
        return value;
    }

    /**
     * Returns a String object representing this Integer's value. The
     * value is converted to signed decimal representation and returned
     * as a string, exactly as if the integer value were given as an
     * argument to the {@link java.lang.Integer#toString(int)} method.
     *
     * @return  a string representation of the value of this object in
     *          base&nbsp;10.
     */
    public String toString() {
        return String.valueOf(value);
    }
}

