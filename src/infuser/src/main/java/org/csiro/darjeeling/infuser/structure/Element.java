/*
 * Element.java
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


/**
 * All information needed to produce infusions, such as class- and method definitions, method implementations, lists, headers
 * imports etc is represented as a tree of Element objects. Operations on this tree are implemented as visitors.   
 * <p>
 * Even though internally the Infuser uses dispatching to separate the different Element types, they are identified with an
 * ElementID, wich is used in the .DI files.    
 * <p>
 * @author Niels Brouwers
 * @see ParentElement ElementId
 *
 */
public abstract class Element implements Comparable<Element>
{
	
	private ElementId id;
	
	protected Element(ElementId id)
	{
		this.id = id;
	}
	
	/**
	 * @return the element ID used to identify different element types. 
	 */
	public ElementId getId()
	{
		return id;
	}
	
	/**
	 * Accept an ElementVisitor 
	 * @param visitor the ElementVisitor instance to visit
	 */
	public abstract void accept(ElementVisitor visitor);

	/**
	 * Implements the Comparable<Element> interface
	 */
	public int compareTo(Element elem)
	{
		if (elem.getId()!=getId())
			return (getId().getId()-elem.getId().getId());
		return (this==elem?0:1);
	}

}
