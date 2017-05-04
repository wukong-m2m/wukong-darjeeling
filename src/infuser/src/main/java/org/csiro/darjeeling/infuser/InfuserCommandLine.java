/*
 * InfuserCommandLine.java
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
 
package org.csiro.darjeeling.infuser;

import java.io.FileNotFoundException;

public class InfuserCommandLine
{
	
	public static void printUsage()
	{
		System.out.println("Usage: infuser [options] file...");
		System.out.println("");
		System.out.println("Options:");
		System.out.println("\t-o=<file>\t\t\t\tOutput infusion file (.di)");
		System.out.println("\t-h=<file>\t\t\t\tOutput header file (.dih)");
		System.out.println("\t-d=<file>\t\t\t\tOutput c definitions header file (.h)");
		System.out.println("\t-name=<arg>\t\t\t\tInfusion name");
		System.out.println("\t-infusionversion=<arg>\t\t\tInfusion version (integer)");
		System.out.println("\t-use32bitarrayindex\t\t\tUse 32 bit array indexes instead of 16 bit");		
		System.out.println("\t-useconstantshiftoptimisation\t\t\tOptimise constant shifts (gcc-style)");		
		System.out.println("");
		System.out.println("Examples:");
		System.out.println("\tinfuser include/sys.dih HelloWorld.class -name=hello -o test.di");
		System.out.println("");
		System.out.println("Files can be java Class files (.class) or darjeeling headers (.dih)");
	}
	
	public static void main(String[] args)
	{
		
		// parse arguments
		InfuserArguments infuserArgs = null;
		try {
			infuserArgs = InfuserArguments.parse(args);
			Infuser infuser = new Infuser(infuserArgs);
			infuser.process();
		} catch (ArgumentParseException ex)
		{
			System.out.println("error: " + ex.getMessage());
			printUsage();
			System.exit(-1);
		} catch (FileNotFoundException ex)
		{
			System.out.println(ex.getMessage());
			System.exit(-1);
		} catch (InfuserException ex)
		{
			System.out.println(ex.getMessage());
			System.exit(-1);
		}
		
	}

}
