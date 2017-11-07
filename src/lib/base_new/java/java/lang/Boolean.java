/*
 * Copyright (c) 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 */
package java.lang;

/**
 * The Boolean class wraps a value of the primitive type
 * <code>boolean</code> in an object. An object of type
 * <code>Boolean</code> contains a single field whose type is
 * <code>boolean</code>.
 *
 * @author  Arthur van Hoff
 * @version 12/17/01 (CLDC 1.1)
 * @since   JDK1.0, CLDC 1.0
 */
public final class Boolean {
    /**
     * The value of the Boolean.
     */
    private boolean value;

    /**
     * Allocates a <code>Boolean</code> object representing the
     * <code>value</code> argument.
     *
     * @param   value   the value of the <code>Boolean</code>.
     */
    public Boolean(boolean value) {
        this.value = value;
    }

    /**
     * Returns the value of this <tt>Boolean</tt> object as a boolean
     * primitive.
     *
     * @return  the primitive <code>boolean</code> value of this object.
     */
    public boolean booleanValue() {
        return value;
    }

    /**
     * Returns a String object representing this Boolean's value.
     * If this object represents the value <code>true</code>, a string equal
     * to <code>"true"</code> is returned. Otherwise, a string equal to
     * <code>"false"</code> is returned.
     *
     * @return  a string representation of this object.
     */
    public String toString() {
        return value ? "true" : "false";
    }
}

