/*
 * AbstractStringTable.java
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
 
package org.csiro.darjeeling.infuser.structure.elements;

import java.util.ArrayList;

import org.csiro.darjeeling.infuser.structure.Element;
import org.csiro.darjeeling.infuser.structure.ElementId;
import org.csiro.darjeeling.infuser.structure.ElementVisitor;
import org.csiro.darjeeling.infuser.structure.GlobalId;

public abstract class AbstractStringTable extends Element
{

	protected ArrayList<String> strings;
	protected AbstractInfusion infusion;

	public AbstractStringTable(AbstractInfusion infusion)
	{
		super(ElementId.STRINGTABLE);
		strings = new ArrayList<String>();
		this.infusion = infusion;
	}

	public GlobalId lookupString(String str)
	{
		GlobalId ret = null;
		
		for (int i=0; i<strings.size(); i++)
			if (str.equals(strings.get(i)))
				return new GlobalId(infusion.getHeader().getInfusionName(), i);
		
		return ret;
	}
	
	public GlobalId addString(String str)
	{
		GlobalId ret = new GlobalId(infusion.getHeader().getInfusionName(), strings.size());
		strings.add(str);
		return ret;
	}
	
	public String[] elements()
	{
		String[] ret = new String[strings.size()];
		strings.toArray(ret);
		return ret;
	}

	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}
	
}
