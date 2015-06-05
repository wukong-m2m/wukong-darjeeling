/*
 * GeneratedValueSet.java
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
 
package org.csiro.darjeeling.infuser.bytecode.analysis;

import java.util.TreeSet;

import org.csiro.darjeeling.infuser.structure.BaseType;

@SuppressWarnings("serial")
public class GeneratedValueSet extends TreeSet<GeneratedValue> implements Comparable<GeneratedValueSet>
{
	
	public BaseType getType()
	{
	 	BaseType ret = null;
	 	
	 	// make sure there is at least a single value in this stack element
	 	if (size()==0)
	 		throw new IllegalStateException("Each stack element must have at least a single value");
		
		for (GeneratedValue handleIndexPair : this)
		{
			BaseType type = handleIndexPair.getOutputType();
			if (ret==null) ret = type; else ret = BaseType.max(ret, type);
		}
		
		return ret;
	}
	
	public BaseType getLogicalType()
	{
	 	// make sure there is at least a single value in this stack element
	 	if (size()==0)
	 		throw new IllegalStateException("Each stack element must have at least a single value");
	 	
	 	BaseType ret = null;
		for (GeneratedValue handleIndexPair : this)
		{
			BaseType type = handleIndexPair.getLogicalOutputType();
			if (ret==null) ret = type; else ret = BaseType.max(ret, type);
		}
		return ret;
	}
	
	public void setOptimizationHint(BaseType baseType)
	{
		for (GeneratedValue handleIndexPair : this)
			handleIndexPair.setOptimizationHint(baseType);
	}
	
	public void setOptimizationHint(BaseType baseType, boolean force)
	{
		for (GeneratedValue handleIndexPair : this)
			handleIndexPair.setOptimizationHint(baseType);
	}
	
	public BaseType getOptimizationHint()
	{
		BaseType ret = null;
		for (GeneratedValue handleIndexPair : this)
			if (ret==null) ret = handleIndexPair.getOptimizationHint();
			else
				ret = BaseType.max(ret, handleIndexPair.getOptimizationHint());
		return ret;
	}
	
	public void setKeepOverflow(boolean keepOverflow)
	{
		for (GeneratedValue handleIndexPair : this)
			handleIndexPair.getHandle().setKeepOverflow(keepOverflow);
	}
	
	public void setGeneratesOverflow(boolean generatesOverflow)
	{
		for (GeneratedValue handleIndexPair : this)
			handleIndexPair.getHandle().setGeneratesOverflow(generatesOverflow);
	}
	
	public static GeneratedValueSet merge(GeneratedValueSet a, GeneratedValueSet b)
	{
		GeneratedValueSet ret = new GeneratedValueSet();
		ret.addAll(a);
		ret.addAll(b);
		return ret;
	}
	
	@Override
	public String toString()
	{
		String ret = "";
		for (GeneratedValue handleIndexPair : this)
			ret += ":" + handleIndexPair.getHandle().getPc() + 
			"[" + handleIndexPair.getIndex() + "]" + 
			handleIndexPair.getOutputType() + 
			"(" + handleIndexPair.getLogicalOutputType() + ")" +
			"<" + handleIndexPair.getOptimizationHint() + ">";
		
		return ret.replaceFirst(":", "");
	}

	public int compareTo(GeneratedValueSet other)
	{
		if (!containsAll(other)) return -1;
		if (!other.containsAll(this)) return 1;
		return 0;
	}

}
