/*
 * HelloWorld.java
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

import javax.rtc.RTCTest;

public class HelloWorld
{
	public static void main(String args[])
	{
		// checkCompareShort0();
		// checkStaticShortVariables();
		// checkStaticInstanceVariables();
		// checkFib();
		// checkCompareShort();
		// checkFib();
		checkIntOps();
	}

	// public static void checkFib()
	// {
	// 	System.out.println("Fibonacci numbers:");
	// 	System.out.println("" + RTCTest.test_fib((short)0));
	// 	System.out.println("" + RTCTest.test_fib((short)1));
	// 	System.out.println("" + RTCTest.test_fib((short)2));
	// 	System.out.println("" + RTCTest.test_fib((short)3));
	// 	System.out.println("" + RTCTest.test_fib((short)4));
	// 	System.out.println("" + RTCTest.test_fib((short)5));
	// 	System.out.println("" + RTCTest.test_fib((short)6));
	// 	System.out.println("" + RTCTest.test_fib((short)7));
	// 	System.out.println("" + RTCTest.test_fib((short)8));
	// 	System.out.println("" + RTCTest.test_fib((short)9));
	// }

	public static void checkIntOps()
	{
		System.out.println("-(-452345) = 452345 " + RTCTest.test_int_ops(-452345, 0, (short)0));
		System.out.println("-(100000) = -100000 " + RTCTest.test_int_ops(100000, 0, (short)0));

		System.out.println("100000+320000 = 420000 " + RTCTest.test_int_ops(100000, 320000, (short)1));
		System.out.println("100000+(-320000) = -220000 " + RTCTest.test_int_ops(100000, -320000, (short)1));

		// broken
		System.out.println("100000-320000 = -220000 " + RTCTest.test_int_ops(100000, 320000, (short)2));
		System.out.println("100000-(-320000) = 420000 " + RTCTest.test_int_ops(100000, -320000, (short)2));

		System.out.println("2*320000 = 640000 " + RTCTest.test_int_ops(2, 320000, (short)3));
		System.out.println("100000*(-3) = -300000 " + RTCTest.test_int_ops(100000, -3, (short)3));

		System.out.println("320000/1000 = 320 " + RTCTest.test_int_ops(320000, 1000, (short)4));
		System.out.println("700000/(-200000) = -3 " + RTCTest.test_int_ops(700000, -200000, (short)4));

		System.out.println("320023 % 100000 = 20023 " + RTCTest.test_int_ops(320023, 100000, (short)5));

		System.out.println("0x0FFF0000 & 0x0F00FF00 = 0x0F000000 = 251658240 " + RTCTest.test_int_ops(0x0FFF0000, 0x0F00FF00, (short)6));
		System.out.println("0x0FFF0000 | 0x0F00FF00 = 0x0FFFFF00 = 268435200 " + RTCTest.test_int_ops(0x0FFF0000, 0x0F00FF00, (short)7));
		System.out.println("0x0FFF0000 ^ 0x0F00FF00 = 0x00FFFF00 = 16776960 " + RTCTest.test_int_ops(0x0FFF0000, 0x0F00FF00, (short)8));
	}

	// public static void checkStaticInstanceVariables()
	// {
	// 	RTCTest obj1 = new RTCTest();
	// 	RTCTest obj2 = new RTCTest();
	// 	System.out.println("test instance1 short1   -1 ->" + obj1.test_instance_short1((short)-1));
	// 	System.out.println("test instance1 short1   42 ->" + obj1.test_instance_short1((short)42));
	// 	System.out.println("test instance1 short1    3 ->" + obj1.test_instance_short1((short)3));
	// 	System.out.println("test instance2 short1    5 ->" + obj2.test_instance_short1((short)5));
	// 	System.out.println("test instance2 short1  -10 ->" + obj2.test_instance_short1((short)-10));
	// 	System.out.println("test instance2 short1    3 ->" + obj2.test_instance_short1((short)3));
	// 	System.out.println("test instance2 short2  300 ->" + obj2.test_instance_short2((short)300));
	// 	System.out.println("test instance2 short2  500 ->" + obj2.test_instance_short2((short)500));
	// 	System.out.println("test instance1 short1 direct access " + obj1.instance_short1);
	// 	System.out.println("test instance2 short1 direct access " + obj2.instance_short1);
	// 	System.out.println("test instance2 short2 direct access " + obj2.instance_short2);
	// }

	// public static void checkStaticShortVariables()
	// {
	// 	System.out.println("test static short -1 ->" + RTCTest.test_static_short((short)-1));
	// 	System.out.println("test static short 42 ->" + RTCTest.test_static_short((short)42));
	// 	System.out.println("test static short  3 ->" + RTCTest.test_static_short((short)3));
	// }

	// public static void checkCompareShort0()
	// {
	// 	System.out.println("-1 EQ" + RTCTest.compare_short_0_EQ((short)-1));
	// 	System.out.println(" 0 EQ" + RTCTest.compare_short_0_EQ((short)0));
	// 	System.out.println(" 1 EQ" + RTCTest.compare_short_0_EQ((short)1));

	// 	System.out.println("-1 NE" + RTCTest.compare_short_0_NE((short)-1));
	// 	System.out.println(" 0 NE" + RTCTest.compare_short_0_NE((short)0));
	// 	System.out.println(" 1 NE" + RTCTest.compare_short_0_NE((short)1));

	// 	System.out.println("-1 LT" + RTCTest.compare_short_0_LT((short)-1));
	// 	System.out.println(" 0 LT" + RTCTest.compare_short_0_LT((short)0));
	// 	System.out.println(" 1 LT" + RTCTest.compare_short_0_LT((short)1));

	// 	System.out.println("-1 LE" + RTCTest.compare_short_0_LE((short)-1));
	// 	System.out.println(" 0 LE" + RTCTest.compare_short_0_LE((short)0));
	// 	System.out.println(" 1 LE" + RTCTest.compare_short_0_LE((short)1));

	// 	System.out.println("-1 GT" + RTCTest.compare_short_0_GT((short)-1));
	// 	System.out.println(" 0 GT" + RTCTest.compare_short_0_GT((short)0));
	// 	System.out.println(" 1 GT" + RTCTest.compare_short_0_GT((short)1));

	// 	System.out.println("-1 GE" + RTCTest.compare_short_0_GE((short)-1));
	// 	System.out.println(" 0 GE" + RTCTest.compare_short_0_GE((short)0));
	// 	System.out.println(" 1 GE" + RTCTest.compare_short_0_GE((short)1));
	// }

	// public static void checkCompareShort()
	// {
	// 	System.out.println("-1 EQ" + RTCTest.compare_short_EQ((short)-101, (short)-100));
	// 	System.out.println(" 0 EQ" + RTCTest.compare_short_EQ((short)-100, (short)-100));
	// 	System.out.println(" 1 EQ" + RTCTest.compare_short_EQ((short)-99, (short)-100));

	// 	System.out.println("-1 NE" + RTCTest.compare_short_NE((short)-101, (short)-100));
	// 	System.out.println(" 0 NE" + RTCTest.compare_short_NE((short)-100, (short)-100));
	// 	System.out.println(" 1 NE" + RTCTest.compare_short_NE((short)-99, (short)-100));

	// 	System.out.println("-1 LT" + RTCTest.compare_short_LT((short)-101, (short)-100));
	// 	System.out.println(" 0 LT" + RTCTest.compare_short_LT((short)-100, (short)-100));
	// 	System.out.println(" 1 LT" + RTCTest.compare_short_LT((short)-99, (short)-100));

	// 	System.out.println("-1 LE" + RTCTest.compare_short_LE((short)-101, (short)-100));
	// 	System.out.println(" 0 LE" + RTCTest.compare_short_LE((short)-100, (short)-100));
	// 	System.out.println(" 1 LE" + RTCTest.compare_short_LE((short)-99, (short)-100));

	// 	System.out.println("-1 GT" + RTCTest.compare_short_GT((short)-101, (short)-100));
	// 	System.out.println(" 0 GT" + RTCTest.compare_short_GT((short)-100, (short)-100));
	// 	System.out.println(" 1 GT" + RTCTest.compare_short_GT((short)-99, (short)-100));

	// 	System.out.println("-1 GE" + RTCTest.compare_short_GE((short)-101, (short)-100));
	// 	System.out.println(" 0 GE" + RTCTest.compare_short_GE((short)-100, (short)-100));
	// 	System.out.println(" 1 GE" + RTCTest.compare_short_GE((short)-99, (short)-100));
	// }
}
