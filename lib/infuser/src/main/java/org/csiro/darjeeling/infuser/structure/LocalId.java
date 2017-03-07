/*
 * LocalId.java
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

import org.csiro.darjeeling.infuser.structure.elements.AbstractInfusion;

public class LocalId
{
	
	public static final int INTARRAY_INF_ID = 254;
	public static final int BUILTIN_INF_ID = 255;

	private int infusionId, entityId; 
	
	public LocalId(int infusionId, int localClassId)
	{
		this.infusionId = infusionId;
		this.entityId = localClassId;
	}
	
	public void translate(AbstractInfusion from, AbstractInfusion to)
	{
		// TODO implement properly, right now we assume that infusion a
		// does not include other infusions, so that all references are local

		String name;
		if (infusionId==0) name = from.getHeader().getInfusionName();
		else
			name = from.getReferencedInfusions().get(infusionId-1).getHeader().getInfusionName();
		
		infusionId = to.getLocalInfusionIdByName(name);
	}
	
	public static LocalId numArray(BaseType type)
	{
		return new LocalId(
				INTARRAY_INF_ID,
				type.getTType()
				);
	}
	
	/**
	 * @return the infusionId
	 */
	public int getInfusionId()
	{
		return infusionId;
	}

	/**
	 * @param infusionId the infusionId to set
	 */
	public void setInfusionId(int infusionId)
	{
		this.infusionId = infusionId;
	}

	/**
	 * @return the localClassId
	 */
	public int getLocalId()
	{
		return entityId;
	}

	/**
	 * @param localClassId the localClassId to set
	 */
	public void setEntityId(int localClassId)
	{
		this.entityId = localClassId;
	}
	
	@Override
	public String toString()
	{
		return String.format("(%d,%d)", infusionId, entityId);
	}

	
	@Override
	public boolean equals(Object other)
	{
		if (!(other instanceof LocalId)) return false;
		LocalId otherId = (LocalId)other;
		return (infusionId==otherId.infusionId) && (entityId==otherId.infusionId);
	}
	
}
