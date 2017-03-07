package org.csiro.darjeeling.infuser.checkphase;

import org.csiro.darjeeling.infuser.logging.Logging;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodDefinitionList;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalMethodImplementationList;

public class ListSizeLimitationsCheckVisitor extends CheckVisitor
{
	
	public void visit(InternalMethodDefinitionList element)
	{
		if (element.size()>255) Logging.instance.error(String.format("Too many method definitions (%d, maximum is 255).", element.size()));

		super.visit(element);
	}
	
	public void visit(InternalMethodImplementationList element)
	{
		if (element.size()>255) Logging.instance.error(String.format("Too many method implementations (%d, maximum is 255).", element.size()));

		super.visit(element);
	}
	

}
