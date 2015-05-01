/*
 * FieldList.java
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
import java.util.List;

import org.csiro.darjeeling.infuser.structure.elements.AbstractField;

/**
 * 
 * Ordered list of fields inside a class.
 * 
 * @author Niels Brouwers
 *
 */
public class FieldList
{

	// Internal list.
	private ArrayList<AbstractField> fields;
	
	/**
	 * Creates a new FieldList
	 */
	public FieldList()
	{
		fields = new ArrayList<AbstractField>();
	}
	
	/**
	 * Adds a field to the list.
	 * @param field field to add.
	 */
	public void addField(AbstractField field)
	{
		fields.add(field);
	}
	
	/**
	 * Does a field look-up by name.
	 * @param name name of the field to find.
	 * @return the field object or null if not found.
	 */
	public AbstractField getFieldByName(String name)
	{
		for (AbstractField field : fields)
			if (field.getName().equals(name))
				return field;
		
		return null;
	}

	/**
	 * Returns the internal arraylist of fields.
	 * @return the internal arraylist of fields.
	 */
	public List<AbstractField> getFields()
	{
		return fields;
	}
	
}
