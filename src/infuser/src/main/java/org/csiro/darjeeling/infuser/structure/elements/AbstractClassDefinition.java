/*
 * AbstractClassDefinition.java
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

import java.util.Collection;
import java.util.HashMap;

import org.csiro.darjeeling.infuser.structure.ElementId;
import org.csiro.darjeeling.infuser.structure.ElementVisitor;
import org.csiro.darjeeling.infuser.structure.FieldList;
import org.csiro.darjeeling.infuser.structure.GlobalId;
import org.csiro.darjeeling.infuser.structure.ParentElement;

public abstract class AbstractClassDefinition extends ParentElement<AbstractMethod>
{
	
	// list of interfaces implemented by this class
	protected HashMap<String, AbstractClassDefinition> interfaces;
	
	// super class
	protected AbstractClassDefinition superClass;
	
	// class fields
	protected FieldList fieldList;
	
	// class assigned global ID
	protected GlobalId globalId;
	
	// class name and superclass' name
	protected String className, superClassName;
	
	// for error reporting
	protected String fileName;
	
	protected AbstractClassDefinition()
	{
		super(ElementId.CLASSDEF);
		fieldList = new FieldList();
		interfaces = new HashMap<String, AbstractClassDefinition>();
	}
	
	public AbstractMethod getMethodByDef(AbstractMethodDefinition methodDef)
	{
		for (AbstractMethod method : getChildren())
			if (AbstractMethodDefinition.getComparator().compare(method.getMethodDef(),methodDef)==0)
				return method;
		
		return null;
	}
	
	public AbstractField getFieldByName(String name)
	{
		AbstractField ret = null;
		
		ret = fieldList.getFieldByName(name);
		if ((ret==null)&&(superClass!=null)) ret = superClass.getFieldByName(name);
		
		return ret;
	}

	public void accept(ElementVisitor visitor)
	{
		visitor.visit(this);
	}

	public void setGlobalId(GlobalId globalId)
	{
		this.globalId = globalId;
	}
	
	public GlobalId getGlobalId()
	{
		return globalId;
	}
	
	/**
	 * @return the className
	 */
	public String getName()
	{
		return className;
	}

	/**
	 * @return the superClassName
	 */
	public String getSuperClassName()
	{
		return superClassName;
	}
	
	/**
	 * Returns the name of the file the class was defined in. Used for error/warning reporting.
	 * @return name of the file the class was defined in
	 */
	public String getFileName()
	{
		return fileName;
	}

	/**
	 * @return the superClass
	 */
	public AbstractClassDefinition getSuperClass()
	{
		return superClass;
	}

	/**
	 * @param superClass the superClass to set
	 */
	public void setSuperClass(AbstractClassDefinition superClass)
	{
		this.superClass = superClass;
	}
	
	public boolean inheritsFrom(AbstractClassDefinition superClass)
	{
		if (this.superClass==superClass) return true;
		if (this.superClass!=null) return this.superClass.inheritsFrom(superClass);
		return false;
	}
	
	/**
	 * @return field list.
	 */
	public FieldList getFieldList()
	{
		return fieldList;
	}
	
	/**
	 * @return the size in bytes of all non-reference fields in this class.
	 */
	public int getNonReferenceFieldsSize()	
	{
		int ret = 0;

		if (superClass!=null)
			ret += superClass.getNonReferenceFieldsSize();
		
		for (AbstractField field : fieldList.getFields())
			if (!field.isRef())
				ret += field.getSize();
		
		return ret;
	}
	
	/**
	 * @return the number of reference-type fields in this class. 
	 */
	public int getReferenceFieldCount()
	{
		int ret = 0;
		
		if (superClass!=null)
			ret += superClass.getReferenceFieldCount();
		
		for (AbstractField field : fieldList.getFields())
			if (field.isRef()) ret ++;
		
		return ret;
	}
	
	/**
	 * @return a list of interfaces implemented by this class.
	 */
	public Collection<String> getInterfaceNames()
	{
		return interfaces.keySet();
	}
	
	/**
	 * @return a list of interfaces implemented by this class.
	 */
	public Collection<AbstractClassDefinition> getInterfaces()
	{
		return interfaces.values();
	}
	
	/**
	 * Adds an interface to the class. To avoid complexity in the VM, the interface list is flattened automatically later in the process.
	 * @param interfaceDefinition interface to be added
	 */
	public void addInterface(AbstractClassDefinition interfaceDefinition)
	{
		// Add the interface itself
		interfaces.put(interfaceDefinition.getName(), interfaceDefinition);
	}
	
	// Helper function for flattenInterfaceList
	private void addInterfaces(AbstractClassDefinition interfaceDef)
	{
		// Add all interfaces implemented by the given interface.
		for (AbstractClassDefinition iDef : interfaceDef.interfaces.values())
			interfaces.put(iDef.getName(), iDef);

		// Add the parent interface of the given interface, if any (recurse).
		if (interfaceDef.getSuperClass()!=null)
			addInterfaces(interfaceDef.getSuperClass());
	}
	
	/**
	 * To avoid complexity in the VM, the set of interfaces implemented by a class is stored as a flat list. This method 'flattens' the 
	 * hierarchy of interfaces by walking the implementation/inheritance tree of each implemented interface. For example, if class C1 implements
	 * interface I1 and I2, and I1 implements I3, then after this method completes the interface list of C1 will include I1, I2, and I3. 
	 */
	public void flattenInterfaceList()
	{
		// Clone interface list to avoid concurrent modification exception
		AbstractClassDefinition[] interfaceList = new AbstractClassDefinition[interfaces.size()];
		interfaces.values().toArray(interfaceList);
		
		// Walk over the interface list and recursively add each interface to the class
		for (AbstractClassDefinition interfaceDefinition : interfaceList)
			addInterfaces(interfaceDefinition);
	}
	
}
