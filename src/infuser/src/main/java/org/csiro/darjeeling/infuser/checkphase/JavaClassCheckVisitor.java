/*
 * JavaClassCheckVisitor.java
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
 
package org.csiro.darjeeling.infuser.checkphase;

import org.csiro.darjeeling.infuser.logging.Logging;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalClassDefinition;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodDefinition;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodImplementation;

public class JavaClassCheckVisitor extends CheckVisitor
{

	public JavaClassCheckVisitor()
	{
	}
	
	public void visit(InternalClassDefinition classDefinition)
	{
		Logging.instance.setCurrentFileName(classDefinition.getFileName());
		
		if (classDefinition.getJavaClass().getMajor()<50)
			Logging.instance.warning("Input class file was compiled with a pre-jdk1.6 compiler. Please compile with jdk 1.6 or greater");

	}
	
	public void visit(InternalMethodImplementation methodImplementation)
	{
		Logging.instance.setCurrentFileName(methodImplementation.getFileName());
		
		if (methodImplementation.isSynchronized())
			Logging.instance.warning(String.format(
					"Method %s is synchronized, which is currently not supported by Darjeeling. Use an explicit synchronized block instead.",
					methodImplementation.getMethodDefinition().getName()
					));
		
	}

	public void visit(InternalMethodDefinition methodDefinition)
	{
	}

}
