/*
 * Copyright (c) 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * 
 */
package java.lang;

/**
 * The class <code>Error</code> and its subclasses are a form of 
 * <code>Throwable</code> that indicates conditions that a reasonable 
 * application might want to catch.
 *
 * @author  Frank Yellin
 * @version 12/17/01 (CLDC 1.1)
 * @see     java.lang.Error
 * @since   JDK1.0, CLDC 1.0
 */
public class Error extends Throwable {

    /**
     * Constructs an <code>Error</code> with no specified detail message. 
     */
    public Error() {
        super();
    }

    /**
     * Constructs an <code>Error</code> with the specified detail message. 
     *
     * @param   s   the detail message.
     */
    public Error(String s) {
        super(s);
    }
}

