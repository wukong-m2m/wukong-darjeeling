/*
 * BaseType.java
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
 
package org.csiro.darjeeling.infuser.structure;

import org.apache.bcel.generic.ArrayType;
import org.apache.bcel.generic.ObjectType;
import org.apache.bcel.generic.Type;

/**
 * 
 * Provides a mapping between Java types and the different base types used in Darjeeling. These can be either integer types
 * such as <i>Byte</i>, <i>Char</i>, <i>Boolean</i>, <i>Short</i>, <i>Int</i>, and the currently unsupported 
 * <i>Float</i>, <i>Long</i>, and <i>Double</i>, or <i>Reference</i>.
 * <p>
 * The <i>size</i> of a BaseType is the number of bytes it takes to store a variable of that type, or -1 for references.   
 * <p>
 * The TType property corresponds to the index given to a basetype in the Java VM spec.
 * <p>
 * @author Niels Brouwers
 *
 */
public enum BaseType
{
	
	Ref(TypeClass.Reference, -1,-1),
	
	Byte(TypeClass.Short, 1, 8),
	Char(TypeClass.Short, 1, 5),
	Boolean(TypeClass.Short, 1, 4),
	
	Short(TypeClass.Short, 2, 9),
	
	Int(TypeClass.Int, 4, 10),
	Float(TypeClass.Int, 4, 6),
	
	Long(TypeClass.Long, 8, 11),
	Double(TypeClass.Long, 8, 7),

	// return type Void
	Void(TypeClass.Void, 0, 0),
	
	// Special cases 
	Unknown(TypeClass.Void, 0, 0),
	DontCare(TypeClass.Void, 0, 0)
	;
	
	private int size, t_type;
	private TypeClass typeClass;
	
	private BaseType(TypeClass typeClass, int size, int t_type)
	{
		this.typeClass = typeClass;
		this.size = size;
		this.t_type = t_type;
	}
	
	/**
	 * Convert a BCEL Type to a Darjeeling base type.
	 * @param type the BCEL Type object to convert
	 * @return a BaseType that corresponds to the BCEL type
	 */
	public static BaseType fromBCELType(Type type)
	{
		if (type==Type.BYTE) return Byte;
		if (type==Type.CHAR) return Char;
		if (type==Type.BOOLEAN) return Boolean;
		if (type==Type.SHORT) return Short;
		if (type==Type.INT) return Int;
		if (type==Type.FLOAT) return Float;
		if (type==Type.LONG) return Long;
		if (type==Type.DOUBLE) return Double;
		
		if (type==Type.OBJECT) return Ref;
		if (type==Type.VOID) return Void;
		
		if (type instanceof ObjectType) return Ref;
		if (type instanceof ArrayType) return Ref;
		
		throw new IllegalStateException("Cannot map BCEL type " + type.getSignature() );
	}
	
	/**
	 * Gets the number of slots on the integer stack that this type takes. Shorts, bytes, booleans 
	 * and chars take up a single slot, integers and floats take two, and doubles and longs take
	 * four.
	 * @return the number of slots on the integer stack that this element takes
	 */
	public int getNrIntegerSlots()
	{
		return typeClass.getNrIntegerSlots();
	}
	
	/**
	 * Gets the number of slots on the reference stack that this type takes. Returns 1 for Ref and 0 for all other
	 * types. 
	 * @return the number of slots on the reference stack that this element takes
	 */
	public int getNrReferenceSlots()
	{
		return typeClass.getNrReferenceSlots();
	}
	
	/**
	 * Gets the Size of the BaseType, as described above. This returns -1 for Ref.
	 * @return the size of the BaseType
	 */
	public int getSize()
	{
		return size;
	}
	
	public TypeClass getTypeClass()
	{
		return typeClass;
	}
	
	/**
	 * Gets the TType of the BaseType, as described above. 
	 * @return the TType of the BaseType
	 */
	public int getTType()
	{
		return t_type;
	}
	
	public boolean isByteSized()
	{
		return (this==Byte)||(this==Boolean)||(this==Char);
	}
	
	public boolean isShortSized()
	{
		return this==Short;
	}
	
	public boolean isIntSized()
	{
		return (this==Int)||(this==Float);
	}
	
	public boolean isLongSized()
	{
		return (this==Long)||(this==Double);
	}
	
	/**
	 * Returns the 'largest' of the two types. The decision is made by size in bytes, so a Short is larger
	 * than a Byte, but smaller than an Int. Note that behavior is undefined for non-integer types.  
	 * @param a a BaseType to compare
	 * @param b a BaseType to compare
	 * @return the largest of the two types or b in case of equal size
	 */
	public static BaseType max(BaseType a, BaseType b)
	{
		return (a.size>b.size)?a:b;
	}
	
	/**
	 * Returns the 'smallest' of the two types. The decision is made by size in bytes, so a Short is larger
	 * than a Byte, but smaller than an Int. Note that behavior is undefined for non-integer types.  
	 * @param a a BaseType to compare
	 * @param b a BaseType to compare
	 * @return the smallest of the two types or b in case of equal size
	 */
	public static BaseType min(BaseType a, BaseType b)
	{
		return (a.size<b.size)?a:b;
	}
	
}
