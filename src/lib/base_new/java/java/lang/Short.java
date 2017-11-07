/*
 * Copyright (c) 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 */
package java.lang;

/**
 * The Short class is the standard wrapper for short values.
 *
 * @author  Nakul Saraiya
 * @version 12/17/01 (CLDC 1.1)
 * @since   JDK1.1, CLDC 1.0
 */
public final class Short {
    /**
     * The value of the Short.
     */
    private short value;

    /**
     * Constructs a Short object initialized to the specified short value.
     *
     * @param value     the initial value of the Short
     */
    public Short(short value) {
        this.value = value;
    }

    /**
     * Returns the value of this Short as a short.
     *
     * @return the value of this Short as a short.
     */
    public short shortValue() {
        return value;
    }

    /**
     * Returns a String object representing this Short's value.
     */
    public String toString() {
        return String.valueOf((int) value);
    }
}

