/*
 * ArrayList.java
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
 
package java.util;

public class ArrayList<E> implements List<E>
{
	
	private short size;
	private short growSize;
	private E elements[];

	public ArrayList(short growSize)
	{
		this.growSize = growSize;
	}
	
	public ArrayList()
	{
		this((short)4);
	}
	

	public E get(short index)
	{
		if (index<0||index>=size)
			throw new IndexOutOfBoundsException();

		return elements[index];
	}

	public short indexOf(E element)
	{
		for (short i=0; i<size; i++)
			if (elements[i]==element) return i;
		
		return -1;
	}

	public E remove(short index)
	{
		if (index<0||index>=size)
			throw new IndexOutOfBoundsException();
		
		E ret = elements[index];
		
		removeAt(index);
		
		return ret;
	}

	public E set(short index, E element)
	{
		if (index<0||index>=size)
			throw new IndexOutOfBoundsException();
		
		E ret = elements[index];
		
		elements[index] = element;
		
		return ret;
	}

	public void add(E element)
	{
		// lazily construct the element array if none was created
		if (elements==null) elements = (E[])(new Object[growSize]);
		
		// check if we need to grow the array
		if (size>=elements.length)
		{
			E[] newElements = (E[])(new Object[size+growSize]);
			// TODO use System.arrayCopy here
			for (int i=0; i<elements.length; i++)
				newElements[i] = elements[i];
			elements = newElements;
		}
		
		// add the new element
		elements[size] = element;
		size++;
	}

	public void clear()
	{
		size = 0;
		elements = null;
	}

	public boolean contains(E element)
	{
		for (int i=0; i<size; i++)
			if (elements[i]==element) return true;
		
		return false;
	}

	public boolean isEmpty()
	{
		return size==0;
	}

	private void removeAt(short index)
	{
		size--;
		for (short i=index; i<size; i++)
			elements[i] = elements[i+1];
	}
	
	public void remove(E element)
	{
		short pos = indexOf(element);
		if (pos!=-1) removeAt(pos);		 
	}

	public short size()
	{
		return size;
	}

	public Object[] toArray()
	{
		E[] ret = (E[])(new Object[size]);
		// TODO System.arrayCopy
		for (int i=0; i<size; i++) ret[i] = elements[i];
		return ret;
	}

	public Iterator<E> iterator()
	{
		return new ArrayListIterator();
	}
	
	private class ArrayListIterator implements Iterator<E>
	{
		private short index;
		
		public boolean hasNext()
		{
			return index<size;
		}

		public E next()
		{
			E ret = (E)elements[index]; 
			index++;
			return ret;
		}

		public void remove()
		{
			removeAt(index);
		}
		
	}
	
	public static short test()
	{
		return 10;
	}

}
