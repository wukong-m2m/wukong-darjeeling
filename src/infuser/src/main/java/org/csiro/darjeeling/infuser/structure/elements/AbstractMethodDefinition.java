/*
 * AbstractMethodDefinition.java
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

import java.util.Comparator;

import org.apache.bcel.generic.Type;
import org.csiro.darjeeling.infuser.structure.BaseType;
import org.csiro.darjeeling.infuser.structure.Element;
import org.csiro.darjeeling.infuser.structure.ElementId;
import org.csiro.darjeeling.infuser.structure.ElementVisitor;
import org.csiro.darjeeling.infuser.structure.GlobalId;

public class AbstractMethodDefinition extends Element 
{
	
	protected GlobalId globalId;
	protected String methodName, descriptor;
	protected static MethodDefinitionComparator comparator;

	public AbstractMethodDefinition(String methodName, String descriptor)
	{
		super(ElementId.METHODDEF);
		this.methodName = methodName;
		this.descriptor = descriptor;
	}
	
	public int getParameterCount()
	{
		return Type.getArgumentTypes(descriptor).length;
	}
	
	public int getReferenceParameterCount()
	{
		int ret = 0;
		for (BaseType type : getArgumentTypes())
			if (type==BaseType.Ref) ret++;
		return ret;
	}
	
	public BaseType getReturnType()
	{
		return BaseType.fromBCELType(Type.getReturnType(descriptor));
	}
	
	public BaseType[] getArgumentTypes()
	{
		Type types[] = Type.getArgumentTypes(descriptor);
		BaseType ret[] = new BaseType[types.length];
		for (int i=0; i<ret.length; i++)
			ret[i] = BaseType.fromBCELType(types[i]);
		return ret;
	}
	
	/**
	 * @return the localMethodId
	 */
	public GlobalId getGlobalId()
	{
		return globalId;
	}

	/**
	 * @param localMethodId the localMethodId to set
	 */
	public void setGlobalId(GlobalId globalId)
	{
		this.globalId = globalId;
	}

	/**
	 * @return the methodName
	 */
	public String getName()
	{
		return methodName;
	}
	
	/**
	 * @return the descriptor
	 */
	public String getSignature()
	{
		return descriptor;
	}

	@Override
	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}
	
	@Override
	public String toString()
	{
		return String.format("%s %s", descriptor, methodName);
	}
	
	public static Comparator<AbstractMethodDefinition> getComparator()
	{
		if (comparator==null)
			comparator = new MethodDefinitionComparator();
		
		return comparator;
	}

	public int compareTo(Element elem)
	{
		if (elem.getId()!=getId())
			return super.compareTo(elem);
		else
			return getComparator().compare(this, (AbstractMethodDefinition)elem);
	}
	
	private static class MethodDefinitionComparator implements Comparator<AbstractMethodDefinition>
	{

		public int compare(AbstractMethodDefinition o1, AbstractMethodDefinition o2)
		{
			
			if ( (o1.getName().equals(o2.getName())) &&
					(o1.getSignature().equals(o2.getSignature())) )
				return 0;
			else
			{
				if ((o1.getGlobalId()==null)||(o2.getGlobalId()==null))
					return -1;
				else
					if (!o1.getGlobalId().getInfusion().equals(o2.getGlobalId().getInfusion()))
						return o2.getGlobalId().getEntityId() - o1.getGlobalId().getEntityId();
					else
						return 1;
			}
			
		}
		
	}

}
