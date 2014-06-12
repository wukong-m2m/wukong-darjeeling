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
import javax.darjeeling.Darjeeling;

public class HelloWorld
{
	public static void main(String args[])
	{
		System.out.println("Hello world.");
		// checkCompareShort0();
		// checkStaticShortVariables();
		// checkStaticIntVariables();
		checkStaticRefVariables();
		// checkInstanceVariables();
		// checkFib();
		// checkCompareShort();
		// checkFib();
		// checkShortOps();
		// checkIntOps();
		// checkBubbleSort();
		// checkMethodCall();
		// checkReturnObject();
		// checkCallingPrintln();
	}

	// public static void checkCallingPrintln() {
	// 	RTCTest.check_calling_println();
	// }

	// public static void checkReturnObject() {
	// 	RTCTest obj = new RTCTest();
	// 	obj.instance_int = 123455;
	// 	RTCTest obj2 = RTCTest.test_return_object(obj);
	// 	System.out.println("123456 " + obj2.instance_int);
	// }

	// public static void checkMethodCall() {
	// 	System.out.println("100109 " + RTCTest.test_method_call_1(10, (short)2, (short)10, 100110));

	// 	RTCTest obj = new RTCTest();
	// 	obj.instance_short1 = 5;
	// 	System.out.println("100114 " + RTCTest.test_method_call_2(10, (short)2, obj, (short)10, 100110));

	// 	System.out.println("100005 " + RTCTest.test_method_call_3(obj));

	// 	obj.instance_int = 654320;
	// 	RTCTest obj2 = RTCTest.test_method_call_4(obj);
	// 	System.out.println("654321 " + obj2.instance_int);		
	// }

	// public static void checkBubbleSort() {
	// 	short NUMNUMBERS = 256;
	// 	short numbers[] = new short[NUMNUMBERS];

	// 	for (int i=0; i<NUMNUMBERS; i++)
	// 		numbers[i] = (short)(NUMNUMBERS - 1 - i);
	// 	RTCTest.test_bubblesort(numbers);
	// 	for (int i=0; i<NUMNUMBERS; i++)
	// 		System.out.print(" " + numbers[i]);
	// 	System.out.println();

	// 	numbers = new short[] { 8924, 6871, 8880, 3847, -691, -7175, -1877, -7482, 161, -4453, -1353, -7387, -178, -9993, 9746, -4730, -5821, 3272, 5597, 2545, 4077, -177, 8040, -7893, 6526, 4943, 7817, -8654, 1791, -2290, -2888, -6876, 2419, 1664, 1401, 9189, -2888, -7559, -3089, 2463, 9195, 2396, 3625, 3054, 9244, 9932, 1780, -8524, 2345, 1323, 1939, 2412, 7854, -5885, 1423, -5392, -7251, -3664, -5654, -3302, 496, -2388, -5971, -5358, -8205, 9127, 9710, 1123, 2101, 2976, 3381, 7523, -2622, -6532, -7377, 3057, 6422, -2660, 534, -9285, 364, 1616, -7795, -4094, 8336, 3635, 821, 209, -4964, -5920, 9552, 9488, 5992, 3250, 192, 5516, -7495, 3987, -2245, -1663 };
	// 	RTCTest.test_bubblesort(numbers);
	// 	for (int i=0; i<numbers.length; i++)
	// 		System.out.print(" " + numbers[i]);
	// 	System.out.println();
	// }

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

	// public static void checkShortOps()
	// {
	// 	System.out.println("-(-4534) = 4534 " + RTCTest.test_short_ops((short)-4534, (short)0, (short)0));
	// 	System.out.println("-(10000) = -10000 " + RTCTest.test_short_ops((short)10000, (short)0, (short)0));

	// 	System.out.println("10000+12000 = 22000 " + RTCTest.test_short_ops((short)10000, (short)12000, (short)1));
	// 	System.out.println("10000+(-32000) = -22000 " + RTCTest.test_short_ops((short)10000, (short)-32000, (short)1));

	// 	System.out.println("10000-32000 = -22000 " + RTCTest.test_short_ops((short)10000, (short)32000, (short)2));
	// 	System.out.println("10000-(-2000) = 12000 " + RTCTest.test_short_ops((short)10000, (short)-2000, (short)2));

	// 	System.out.println("2*3200 = 6400 " + RTCTest.test_short_ops((short)2, (short)3200, (short)3));
	// 	System.out.println("1000*(-3) = -3000 " + RTCTest.test_short_ops((short)1000, (short)-3, (short)3));

	// 	System.out.println("32000/100 = 320 " + RTCTest.test_short_ops((short)32000, (short)100, (short)4));
	// 	System.out.println("7000/(-2000) = -3 " + RTCTest.test_short_ops((short)7000, (short)-2000, (short)4));

	// 	System.out.println("32023 % 10000 = 2023 " + RTCTest.test_short_ops((short)32023, (short)10000, (short)5));

	// 	System.out.println("0x0FF0 & 0x00FF = 0x00F0 = 240 " + RTCTest.test_short_ops((short)0x0FF0, (short)0x00FF, (short)6));
	// 	System.out.println("0x0FF0 | 0x00FF = 0x0FFF = 4095 " + RTCTest.test_short_ops((short)0x0FF0, (short)0x00FF, (short)7));
	// 	System.out.println("0x0FF0 ^ 0x00FF = 0x0F0F = 3855 " + RTCTest.test_short_ops((short)0x0FF0, (short)0x00FF, (short)8));

	// 	System.out.println("42 + (-1) = 41 " + RTCTest.test_short_ops((short)42, (short)0, (short)9));
	// 	System.out.println("42 + (42) = 84 " + RTCTest.test_short_ops((short)42, (short)0, (short)10));

	// 	System.out.println("0x1 << 4 = 0x10 = 16 " + RTCTest.test_short_ops((short)1, (short)4, (short)11));
	// 	System.out.println("-1 (0xFFFF) >>  8 = 0xFFFF = -1 " + RTCTest.test_short_ops((short)-1, (short)8, (short)12));
	// 	System.out.println("-1 (0xFFFF) >>> 8 = 0xFF = 255 " + RTCTest.test_short_ops((short)-1, (short)8, (short)13));
	// 	System.out.println("-0xFF0 (0xF010) >> 4 = 0xFF01 = -0xFF = -255 " + RTCTest.test_short_ops((short)-0xFF0, (short)4, (short)12));
	// 	System.out.println("-0xFF0 (0xF010) >>> 4 = 0x0F01 = 3841 " + RTCTest.test_short_ops((short)-0xFF0, (short)4, (short)13));
	// }

	// public static void checkIntOps()
	// {
	// 	System.out.println("-(-452345) = 452345 " + RTCTest.test_int_ops(-452345, 0, (short)0));
	// 	System.out.println("-(100000) = -100000 " + RTCTest.test_int_ops(100000, 0, (short)0));

	// 	System.out.println("100000+320000 = 420000 " + RTCTest.test_int_ops(100000, 320000, (short)1));
	// 	System.out.println("100000+(-320000) = -220000 " + RTCTest.test_int_ops(100000, -320000, (short)1));

	// 	System.out.println("100000-320000 = -220000 " + RTCTest.test_int_ops(100000, 320000, (short)2));
	// 	System.out.println("100000-(-320000) = 420000 " + RTCTest.test_int_ops(100000, -320000, (short)2));

	// 	System.out.println("2*320000 = 640000 " + RTCTest.test_int_ops(2, 320000, (short)3));
	// 	System.out.println("100000*(-3) = -300000 " + RTCTest.test_int_ops(100000, -3, (short)3));

	// 	System.out.println("320000/1000 = 320 " + RTCTest.test_int_ops(320000, 1000, (short)4));
	// 	System.out.println("700000/(-200000) = -3 " + RTCTest.test_int_ops(700000, -200000, (short)4));

	// 	System.out.println("320023 % 100000 = 20023 " + RTCTest.test_int_ops(320023, 100000, (short)5));

	// 	System.out.println("0x0FFF0000 & 0x0F00FF00 = 0x0F000000 = 251658240 " + RTCTest.test_int_ops(0x0FFF0000, 0x0F00FF00, (short)6));
	// 	System.out.println("0x0FFF0000 | 0x0F00FF00 = 0x0FFFFF00 = 268435200 " + RTCTest.test_int_ops(0x0FFF0000, 0x0F00FF00, (short)7));
	// 	System.out.println("0x0FFF0000 ^ 0x0F00FF00 = 0x00FFFF00 = 16776960 " + RTCTest.test_int_ops(0x0FFF0000, 0x0F00FF00, (short)8));

	// 	System.out.println("42 + (-1) = 41 " + RTCTest.test_int_ops(42, 0, (short)9));
	// 	System.out.println("42 + (42) = 84 " + RTCTest.test_int_ops(42, 0, (short)10));

	// 	System.out.println("0x1 << 4 = 0x10 = 16 " + RTCTest.test_int_ops(1, 4, (short)11));
	// 	System.out.println("-1 (0xFFFFFFFF) >>  24 = 0xFFFFFFFF = -1 " + RTCTest.test_int_ops(-1, 24, (short)12));
	// 	System.out.println("-1 (0xFFFFFFFF) >>> 24 = 0xFF = 255 " + RTCTest.test_int_ops(-1, 24, (short)13));
	// 	System.out.println("-0xFF00 (0xFFFF0100) >> 8 = 0xFFFFFF01 = -0xFF = -255 " + RTCTest.test_int_ops(-0xFF00, 8, (short)12));
	// 	System.out.println("-0xFF00 (0xFFFF0100) >>> 8 = 0x00FFFF01 = 16776961 " + RTCTest.test_int_ops(-0xFF00, 8, (short)13));
	// }

	// public static void checkInstanceVariables()
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
	// 	System.out.println("test static short -1 -> " + RTCTest.test_static_short((short)-1));
	// 	System.out.println("test static short 42 -> " + RTCTest.test_static_short((short)42));
	// 	System.out.println("test static short  3 -> " + RTCTest.test_static_short((short)3));

	// 	Darjeeling.rtc_test_short1 = 101;
	// 	Darjeeling.rtc_test_short2 = 102;

	// 	System.out.println("Darjeeling.rtc_test_short1 = " + Darjeeling.rtc_test_short1);
	// 	System.out.println("Darjeeling.rtc_test_short2 = " + Darjeeling.rtc_test_short2);
	// 	System.out.println("test static short in other infusion(1) -1 -> 101-1=100 " + RTCTest.test_static_short_in_other_infusion_1((short)-1));
	// 	System.out.println("Darjeeling.rtc_test_short1 = " + Darjeeling.rtc_test_short1);
	// 	System.out.println("test static short in other infusion(1) 42 -> 101-1+42=142 " + RTCTest.test_static_short_in_other_infusion_1((short)42));
	// 	System.out.println("Darjeeling.rtc_test_short1 = " + Darjeeling.rtc_test_short1);
	// 	System.out.println("test static short in other infusion(2) 3 -> 102+3=105 " + RTCTest.test_static_short_in_other_infusion_2((short)3));
	// 	System.out.println("Darjeeling.rtc_test_short2 = " + Darjeeling.rtc_test_short2);
	// 	System.out.println("test static short in other infusion(1) 10 -> 101-1+42+10=152 " + RTCTest.test_static_short_in_other_infusion_1((short)10));
	// 	System.out.println("Darjeeling.rtc_test_short1 = " + Darjeeling.rtc_test_short1);
	// 	System.out.println("test static short in other infusion(2)  3 -> 102+3+3=108 " + RTCTest.test_static_short_in_other_infusion_2((short)3));
	// 	System.out.println("Darjeeling.rtc_test_short2 = " + Darjeeling.rtc_test_short2);
	// }

	// public static void checkStaticIntVariables()
	// {
	// 	System.out.println("test static int -1 -> " + RTCTest.test_static_int(-1));
	// 	System.out.println("test static int 100042 -> " + RTCTest.test_static_int(100042));
	// 	System.out.println("test static int 3 -> " + RTCTest.test_static_int(3));

	// 	Darjeeling.rtc_test_int1 = 100101;
	// 	Darjeeling.rtc_test_int2 = 100102;

	// 	System.out.println("Darjeeling.rtc_test_int1 = " + Darjeeling.rtc_test_int1);
	// 	System.out.println("Darjeeling.rtc_test_int2 = " + Darjeeling.rtc_test_int2);
	// 	System.out.println("test static int in other infusion(1) -1 -> 100101-1=100100 " + RTCTest.test_static_int_in_other_infusion_1(-1));
	// 	System.out.println("Darjeeling.rtc_test_int1 = " + Darjeeling.rtc_test_int1);
	// 	System.out.println("test static int in other infusion(1) 42 -> 100101-1+42=100142 " + RTCTest.test_static_int_in_other_infusion_1(42));
	// 	System.out.println("Darjeeling.rtc_test_int1 = " + Darjeeling.rtc_test_int1);
	// 	System.out.println("test static int in other infusion(2) 3 -> 100102+3=100105 " + RTCTest.test_static_int_in_other_infusion_2(3));
	// 	System.out.println("Darjeeling.rtc_test_int2 = " + Darjeeling.rtc_test_int2);
	// 	System.out.println("test static int in other infusion(1) 10 -> 100101-1+42+100000=200142 " + RTCTest.test_static_int_in_other_infusion_1(100000));
	// 	System.out.println("Darjeeling.rtc_test_int1 = " + Darjeeling.rtc_test_int1);
	// 	System.out.println("test static int in other infusion(2)  3 -> 100102+3+3=100108 " + RTCTest.test_static_int_in_other_infusion_2(3));
	// 	System.out.println("Darjeeling.rtc_test_int2 = " + Darjeeling.rtc_test_int2);
	// }

	public static void checkStaticRefVariables()
	{
		RTCTest.static_ref1 = new RTCTest();
		RTCTest.static_ref1.instance_short1 = 101;
		RTCTest.static_ref2 = new RTCTest();
		RTCTest.static_ref2.instance_short1 = 102;

		System.out.println("RTCTest.static_ref1.instance_short1 = " + RTCTest.static_ref1.instance_short1);
		System.out.println("RTCTest.static_ref2.instance_short1 = " + RTCTest.static_ref2.instance_short1);
		System.out.println("Calling RTCTest.test_static_ref_swap()");
		RTCTest.test_static_ref_swap();
		System.out.println("RTCTest.static_ref1.instance_short1 = " + RTCTest.static_ref1.instance_short1);
		System.out.println("RTCTest.static_ref2.instance_short1 = " + RTCTest.static_ref2.instance_short1);
	}

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
