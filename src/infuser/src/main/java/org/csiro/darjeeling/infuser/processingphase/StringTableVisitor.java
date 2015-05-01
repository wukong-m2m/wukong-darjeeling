package org.csiro.darjeeling.infuser.processingphase;

import org.csiro.darjeeling.infuser.structure.DescendingVisitor;
import org.csiro.darjeeling.infuser.structure.Element;
import org.csiro.darjeeling.infuser.structure.elements.AbstractClassDefinition;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalClassDefinition;
import org.csiro.darjeeling.infuser.structure.elements.internal.InternalInfusion;

public class StringTableVisitor extends DescendingVisitor
{
	
	private InternalInfusion infusion;
	
	public StringTableVisitor(InternalInfusion infusion)
	{
		this.infusion = infusion;
		
	}
	
	@Override
	public void visit(InternalClassDefinition element)
	{
		AbstractClassDefinition throwable = this.infusion.lookupClassByName("java.lang.Throwable");
		
		if (element.inheritsFrom(throwable) || element==throwable)
			element.setNameId(infusion.getStringTable().addString(element.getName()));
		else
			element.setNameId(infusion.getStringTable().addString("OBJECT"));
	}
	
	@Override
	public void visit(Element element)
	{
	}
	

}
