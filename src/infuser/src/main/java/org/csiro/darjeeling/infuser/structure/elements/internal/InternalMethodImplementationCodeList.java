/*
 * InternalMethodImplementationList.java
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
 
package org.csiro.darjeeling.infuser.structure.elements.internal;

import java.util.Collection;
import org.csiro.darjeeling.infuser.structure.ElementVisitor;
import org.csiro.darjeeling.infuser.structure.ParentElement;
import org.csiro.darjeeling.infuser.structure.ElementId;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodImplementation;

public class InternalMethodImplementationCodeList extends ParentElement<AbstractMethodImplementation>
{
	// This is a bit of an ugly hack to get the infuser to separate method headers and implementation.
	// InternalMethodImplementationCodeList is a wrapper around the normal InternalMethodImplementationList
	// which actually contains the methods. Normally it isn't processed, since ElementVisitor will ignore it.
	// But DIWriterVisitor will use it to determine whether it needs to write method headers or code.
	// Normally it will just output the method headers, but when InternalMethodImplementationCodeList
	// is processed it will set a static variable and process the method implementation elements
	// a second time, this time emitting the code instead of headers. Afterwards the variable is reset to
	// emit headers for the next infusion.
	InternalMethodImplementationList implList;

	public InternalMethodImplementationCodeList(InternalMethodImplementationList implList) {
		super(ElementId.METHODIMPLCODELIST);
		this.implList = implList;
	}
	
	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}

	@Override
	public Collection<AbstractMethodImplementation> getChildren()
	{
		return implList.getChildren();
	}
}
