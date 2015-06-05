/*
 * InfusionTest.java
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
 
package testvm.tests;

import javax.darjeeling.Darjeeling;
import javax.darjeeling.vm.Infusion;
import javax.darjeeling.vm.InfusionUnloadDependencyException;

public class InfusionTest
{
	
//	public static void printInfusion(String prefix, Infusion infusion)
//	{
//		Darjeeling.print(prefix);
//		Darjeeling.print(infusion.getName());
//		Darjeeling.print("\n");
//		
//		// iterate over imported infusion list
//		for (short j=0; j<infusion.getImportedInfusionCount(); j++)
//		{
//			Infusion importedInfusion = infusion.getImportedInfusion(j);
//			printInfusion(String.concat(prefix, "\t"), importedInfusion);
//		}
//	}
	
	public static void test(int testBase)
	{
		
		Infusion base = Infusion.getInfusionByName("base");
		
		Darjeeling.assertTrue(testBase + 0, base != null);
		Darjeeling.assertTrue(testBase + 1, (base!=null) && (base.getImportedInfusionCount()==0));
		
		try {
			base.unload();
			Darjeeling.assertTrue(testBase + 2, false);
		} catch (InfusionUnloadDependencyException ex)
		{
			Darjeeling.assertTrue(testBase + 2, true);
		}
		
		Infusion testSuite = Infusion.getInfusionByName("testsuite");
		try {
			testSuite.unload();
			Darjeeling.assertTrue(testBase + 3, false);
		} catch (InfusionUnloadDependencyException ex)
		{
			Darjeeling.assertTrue(testBase + 3, false);
		}
		
	}

}
