/*
 * ClassInitialiserResolutionVisitor.java
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
 
package org.csiro.darjeeling.infuser.processingphase;

import org.csiro.darjeeling.infuser.structure.DescendingVisitor;
import org.csiro.darjeeling.infuser.structure.Element;
import org.csiro.darjeeling.infuser.structure.elements.AbstractInfusion;
import org.csiro.darjeeling.infuser.structure.elements.AbstractMethodImplementation;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalClassDefinition;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodImplementation;

/**
 * 
 * This visitor links Class definitions and clinit methods (class initialisers). Class initialisers are run when the infusion is loaded
 * to initialise things like static variables (i.e. public static int bla = 10;).  
 * 
 * @author Niels Brouwers
 *
 */
public class ClassInitialiserResolutionVisitor extends DescendingVisitor
{
	
	// Infusion context
	private AbstractInfusion infusion;
	
	/**
	 * Constructs a new instance. 
	 * @param infusion the Infusion context we're working in
	 */
	public ClassInitialiserResolutionVisitor(AbstractInfusion infusion)
	{
		this.infusion = infusion;
	}
	
	public void visit(InternalClassDefinition element)
	{
		// run over the method implementation list and look for methods with the <clinit> name
		for (AbstractMethodImplementation methodImplementation : infusion.getMethodImplementationList().getChildren())
		{
			// if we find a clinit method, whose parent class equals the class we're inspecting, link them together in the class definition  
			if (methodImplementation.getMethodDefinition().getName().equals("<clinit>")&&(methodImplementation.getParentClass()==element))
				element.setCInit((InternalMethodImplementation)methodImplementation);
		}
	}

	public void visit(Element element)
	{
	}

}
