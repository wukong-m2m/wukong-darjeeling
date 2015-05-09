/*
 * ArrayQueue.java
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
 
package org.csiro.messaging;

public class ArrayQueue<T> {

	protected final T[] queue;
	protected byte head, size;

	public ArrayQueue(int size) {
		queue = (T[]) (new Object[size]);
		head = 0;
		size = 0;
	}

	public boolean offer(T element) {
		if (size >= queue.length)
			return false;

		queue[(head + size) % queue.length] = element;
		size++;

		return true;
	}

	public T peek() {
		if (size == 0)
			return null;
		else
			return queue[head];
	}

	public T peek(short index) {
		if (index >= size)
			throw new IndexOutOfBoundsException();
		else
			return queue[(head + index) % queue.length];
	}

	public T poll() {
		if (size == 0)
			return null;
		else {
			T ret = queue[head];
			queue[head] = null;
			head = (byte) ((head + 1) % queue.length);
			size--;
			return ret;
		}
	}

	public void remove() {
		if (size > 0) {
			queue[head] = null;
			head = (byte) ((head + 1) % queue.length);
			size--;
		}
	}

	public short numElements() {
		return size;
	}

}
