/*
 * Bag.java
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
 
package javax.util;

import java.util.Collection;

/**
 * 
 * A <i>bag</i> or <i>multiset</i> is a collection where each element can appear more than one time, and where the order of
 * elements doesn't matter. 
 * <p>
 * This interface adds nothing to the Collection interface, but simply acts as a way to further group implementations.    
 * 
 * @author Niels Brouwers
 *
 */
public interface Bag<E> extends Collection<E>
{
	
	public E get(short index);

}
