/*
 * System.java
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

import java.io.PrintStream;

public class System 
{
	
	public static PrintStream out = new PrintStream(null);

	// no instance allowed
	private System() {};
	
    public static native long currentTimeMillis();
    
    public static native void arraycopy(Object src, int src_position, Object dst, int dst_position, int length);

    public static void gc()
    {
    	Runtime.getRuntime().gc();
    }

//	public static void setOut(PrintStream printStream) {
//		out = printStream;
//	}


}
