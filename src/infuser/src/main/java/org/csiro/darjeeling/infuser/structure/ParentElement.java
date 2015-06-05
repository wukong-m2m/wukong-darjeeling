/*
 * ParentElement.java
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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Comparator;
import java.util.Iterator;


public abstract class ParentElement<T extends Element> extends Element
{
	
	protected ArrayList<T> childElements;
	private Comparator<T> comparator;
	
	public ParentElement(ElementId id)
	{
		super(id);
		childElements = new ArrayList<T>();
		this.comparator = null;
	}

	public ParentElement(ElementId id, Comparator<T> comparator)
	{
		super(id);
		childElements = new ArrayList<T>();
		this.comparator = comparator;
	}

	/**
	 * @param arg0
	 * @return
	 * @see java.util.ArrayList#add(java.lang.Object)
	 */
	public T add(T newElement)
	{
		if (comparator!=null)
		{
			for (T element : childElements)
				if (comparator.compare(element, newElement)==0)
					return element;
		} else {
			for (T element : childElements)
				if (element.compareTo(newElement)==0)
					return element;
		}
		
		childElements.add(newElement);
		
		return newElement;
	}

	/**
	 * 
	 * @see java.util.ArrayList#clear()
	 */
	public void clear()
	{
		childElements.clear();
	}

	/**
	 * @param arg0
	 * @return
	 * @see java.util.ArrayList#remove(java.lang.Object)
	 */
	public boolean remove(T arg0)
	{
		return childElements.remove(arg0);
	}
	
	

	/**
	 * @return
	 * @see java.util.Set#iterator()
	 */
	public Iterator<T> iterator()
	{
		return childElements.iterator();
	}

	/**
	 * @param arg0
	 * @return
	 * @see java.util.Set#toArray(T[])
	 */
	public T[] toArray(T[] arg0)
	{
		return childElements.toArray(arg0);
	}

	/**
	 * @return
	 * @see java.util.ArrayList#size()
	 */
	public int size()
	{
		return childElements.size();
	}
	
	public Collection<T> getChildren()
	{
		return childElements;
	}
	
	public T get(int index)
	{
		return childElements.get(index);
	}
	
	public abstract void accept(ElementVisitor visitor);

}
