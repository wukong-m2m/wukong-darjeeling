/*
 * GlobalId.java
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

public class GlobalId
{
	
	private String infusion;
	private int entityId;
	
	public GlobalId(String infusion, int entityId)
	{
		this.infusion = infusion;
		this.entityId = entityId;
	}
	
	public LocalId resolve(AbstractInfusion infusion)
	{
		// resolve infusion by name
		return new LocalId(infusion.getLocalInfusionIdByName(this.infusion),entityId);
	}
	
	public int getEntityId()
	{
		return entityId;
	}
	
	public String getInfusion()
	{
		return infusion;
	}
	
	public void setEntityId(int entityId)
	{
		this.entityId = entityId;
	}
	
	public void setInfusion(String infusion)
	{
		this.infusion = infusion;
	}
	
	public String toString()
	{
		return String.format("(%s,%d)", infusion, entityId);
	}

}
