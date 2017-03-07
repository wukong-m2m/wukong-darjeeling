/*
 * ExternalInfusion.java
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
 
package org.csiro.darjeeling.infuser.structure.elements.external;

import org.csiro.darjeeling.infuser.structure.ElementVisitor;
import org.csiro.darjeeling.infuser.structure.elements.AbstractInfusion;
import org.w3c.dom.Document;

public class ExternalInfusion extends AbstractInfusion
{

	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}
	
	public static ExternalInfusion fromDocument(Document doc, String fileName)
	{
		ExternalInfusion ret = new ExternalInfusion();

		// read Header
		ret.setHeader(ExternalHeader.fromDocument(doc));

		// read class list
		ret.setClassList(ExternalClassList.fromDocument(doc, ret, fileName));

		// read method definition list
		ret.setMethodDefinitionList(ExternalMethodDefinitionList.fromDocument(doc, ret));

		// read method implementation list
		ret.setMethodImplementationList(ExternalMethodImplementationList.fromDocument(doc, ret, fileName));

		// read static field list
		ret.setStaticFieldList(ExternalStaticFieldList.fromDocument(doc, ret));
		
		// read string table
		ret.setStringTable(ExternalStringTable.fromDocument(doc, ret));
		
		// read imported infusions
		ret.referencedInfusionList = ExternalReferencedInfusionList.fromDocument(doc);

		return ret;
	}

}
