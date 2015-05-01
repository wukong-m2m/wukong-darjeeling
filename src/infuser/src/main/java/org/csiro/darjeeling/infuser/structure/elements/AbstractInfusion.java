/*
 * AbstractInfusion.java
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

import org.csiro.darjeeling.infuser.structure.Element;
import org.csiro.darjeeling.infuser.structure.ElementId;
import org.csiro.darjeeling.infuser.structure.ElementVisitor;
import org.csiro.darjeeling.infuser.structure.GlobalId;
import org.csiro.darjeeling.infuser.structure.ParentElement;

public class AbstractInfusion extends ParentElement<Element>
{

	protected AbstractClassList classList;
	protected AbstractMethodDefinitionList methodDefinitionList;
	protected AbstractMethodImplementationList methodImplementationList;
	protected AbstractStaticFieldList staticFieldList;
	protected AbstractHeader header;
	protected AbstractStringTable stringTable;
	protected AbstractReferencedInfusionList referencedInfusionList;

	protected AbstractInfusion()
	{
		super(ElementId.INFUSION);
	}

	public AbstractClassList getClassList()
	{
		return classList;
	}

	public AbstractHeader getHeader()
	{
		return header;
	}

	public AbstractMethodImplementationList getMethodImplementationList()
	{
		return methodImplementationList;
	}

	public AbstractStaticFieldList getStaticFieldList()
	{
		return staticFieldList;
	}

	public AbstractMethodDefinitionList getMethodDefinitionList()
	{
		return methodDefinitionList;
	}

	public AbstractReferencedInfusionList getReferencedInfusions()
	{
		return referencedInfusionList;
	}
	
	public AbstractStringTable getStringTable()
	{
		return stringTable;
	}

	public void setHeader(AbstractHeader header)
	{
		if (!childElements.contains(header))
			childElements.add(header);
		this.header = header;
	}

	public void setClassList(AbstractClassList classList)
	{
		if (!childElements.contains(classList))
			childElements.add(classList);
		this.classList = classList;
	}

	public void setMethodDefinitionList(AbstractMethodDefinitionList methodDefList)
	{
		if (!childElements.contains(methodDefList))
			childElements.add(methodDefList);
		this.methodDefinitionList = methodDefList;
	}
	
	public void setMethodImplementationList(
			AbstractMethodImplementationList methodImplementationList)
	{
		if (!childElements.contains(methodImplementationList))
			childElements.add(methodImplementationList);
		this.methodImplementationList = methodImplementationList;
	}

	public void setStaticFieldList(
			AbstractStaticFieldList staticFieldList)
	{
		if (!childElements.contains(staticFieldList))
			childElements.add(staticFieldList);
		this.staticFieldList = staticFieldList;
	}
	
	public void setStringTable(AbstractStringTable stringTable)
	{
		// TODO wtf? :)
		if (!childElements.contains(stringTable))
			childElements.add(stringTable);
		this.stringTable = stringTable;
	}

	public AbstractMethodDefinition getMethodDefinition(GlobalId globalId)
	{
		
		AbstractInfusion infusion = getLocalInfusionByName(globalId.getInfusion());
		return infusion.getMethodDefinitionList().get(globalId.getEntityId());
	}

	public AbstractMethodImplementation getMethodImplementation(GlobalId globalId)
	{
		AbstractInfusion infusion = getLocalInfusionByName(globalId.getInfusion());
		return infusion.getMethodImplementationList().get(globalId.getEntityId());
	}

	public AbstractClassDefinition getClassDefinition(GlobalId globalId)
	{
		AbstractInfusion infusion = getLocalInfusionByName(globalId.getInfusion());
		return infusion.getClassList().get(globalId.getEntityId());
	}

	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}
	
	public AbstractMethodImplementation lookupMethodImplemention(String methodName, String methodSignature, String className)
	{
		// look for the method in the method implementation list
		AbstractMethodImplementation ret = methodImplementationList.lookupMethod(methodName, methodSignature, className);

		// look in other referenced infusions
		if ((ret==null)&&(referencedInfusionList != null))
			for (AbstractInfusion infusion : referencedInfusionList.getChildren())
			{
				ret = infusion.methodImplementationList.lookupMethod(methodName, methodSignature, className);
				if (ret != null) break;
			}

		return ret;

	}

	public AbstractMethodDefinition lookupMethodDefinition(String name, String signature)
	{
		// look for the method in the method implementation list
		AbstractMethodDefinition ret = methodDefinitionList.lookup(name, signature);

		// look in other referenced infusions
		if (referencedInfusionList != null)
			for (AbstractInfusion infusion : referencedInfusionList.getChildren())
				if (ret == null)
					ret = infusion.getMethodDefinitionList().lookup(name, signature);

		return ret;

	}

	public AbstractClassDefinition lookupClassByName(String className)
	{
		AbstractClassDefinition ret;

		// first check the local class definitions
		ret = classList.lookupClassByName(className);

		// look in other referenced infusions
		if (referencedInfusionList != null)
			for (AbstractInfusion infusion : referencedInfusionList.getChildren())
				if (ret == null)
					ret = infusion.lookupClassByName(className);

		return ret;
	}

	public GlobalId lookupString(String str)
	{
		GlobalId ret;

		// first check the local class definitions
		ret = stringTable.lookupString(str);

		// look in other referenced infusions
		if (referencedInfusionList != null)
			for (AbstractInfusion infusion : referencedInfusionList.getChildren())
				if (ret == null)
					ret = infusion.lookupString(str);

		return ret;
	}

	public AbstractField lookupStaticField(String className, String fieldName, String descriptor)
	{
		AbstractField ret;

		ret = staticFieldList.findField(className, fieldName, descriptor);

		if (referencedInfusionList != null)
			for (AbstractInfusion infusion : referencedInfusionList.getChildren())
				if (ret == null)
					ret = infusion.getStaticFieldList().findField(className, fieldName, descriptor);

		return ret;
	}

	public int getLocalInfusionIdByName(String name)
	{
		// is root infusion ?
		if (this.getHeader().getInfusionName().equals(name))
			return 0;

		// referenced infusions
		int i = 1;
		for (AbstractInfusion infusion : referencedInfusionList.getChildren())
		{
			if (infusion.getHeader().getInfusionName().equals(name))
				return i;
			i++;
		}

		throw new IllegalStateException(String.format("Infusion %s not included, should be checked in phase 1!",name));
	}

	public AbstractInfusion getLocalInfusionByName(String name)
	{
		// is root infusion ?
		if (this.getHeader().getInfusionName().equals(name))
			return this;

		// referenced infusions
		for (AbstractInfusion infusion : referencedInfusionList.getChildren())
			if (infusion.getHeader().getInfusionName().equals(name))
				return infusion;

		throw new IllegalStateException(String.format("Infusion %s not included, should be checked in phase 1!",name));
	}

	public void addInfusion(AbstractInfusion infusion)
	{
		referencedInfusionList.add(infusion);
	}

}
