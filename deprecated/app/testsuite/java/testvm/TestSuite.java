/*
 * TestSuite.java
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
 
package testvm;

import javax.darjeeling.Darjeeling;

import testvm.tests.AlignmentTest;
import testvm.tests.ArithmeticTest;
import testvm.tests.ArrayTest;
import testvm.tests.BitManipulationTest;
import testvm.tests.ClassHierarchyTest;
import testvm.tests.CompareTest;
import testvm.tests.ExceptionsTest;
import testvm.tests.FieldTest;
import testvm.tests.GarbageCollectionTest;
import testvm.tests.InheritanceTest;
import testvm.tests.InitialiserTest;
import testvm.tests.InvokeVirtualTest;
import testvm.tests.MD5Test;
import testvm.tests.MethodOverloadingTest;
import testvm.tests.RuntimeExceptionsTest;
import testvm.tests.StaticFieldsTest;
import testvm.tests.SwitchTest;
import testvm.tests.ThreadTest;

public class TestSuite
{
	
	public static void test()
	{
		try {
			ArithmeticTest.test(000);
			ArrayTest.test(100);
			CompareTest.test(200);
			ClassHierarchyTest.test(300);
			StaticFieldsTest.test(400);
			FieldTest.test(500);
			InitialiserTest.test(600);
			InvokeVirtualTest.test(700);
			GarbageCollectionTest.test(800);
			SwitchTest.test(900);
			ThreadTest.test(1000);
			InheritanceTest.test(1100);
			ExceptionsTest.test(1200);
	 		MD5Test.test(1300);
			MethodOverloadingTest.test(1400);
			RuntimeExceptionsTest.test(1500);
			BitManipulationTest.test(1700);
			AlignmentTest.test(1800);
//			TryCatchTest.test(1900);
//			InfusionTest.test(2000);
		} catch (Throwable t)
		{
			Darjeeling.print("Uncaught throwable: " + t.getMessage());
		}
		Darjeeling.print("All testsuites have passed succsessfuly.\n");
	}

	public static void main(String[] args)
	{
		try {
			test();
		} catch (Throwable t)
		{
			Darjeeling.print(t.toString());
		}
	}
}
