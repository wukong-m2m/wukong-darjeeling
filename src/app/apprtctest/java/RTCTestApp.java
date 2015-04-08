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

import javax.rtc.*;
import javax.rtctest.*;
import javax.darjeeling.Stopwatch;

public class RTCTestApp
{
	public static void main(String args[])
	{
		// System.out.println("Hello world.");
		checkReturnValues();
		// checkCompareShort();
		// checkCompareInt();
		// checkCompareShort0();
		// checkCompareInt0();
		// checkStaticShortVariables();
		// checkStaticIntVariables();
		// checkStaticRefVariables();
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
		// checkNew();
		// checkObjTests();
		// checkSomeIntOperations();
		// checkNewArray();
		// checkNewObjectArray();
		// checkCharByteObjStaticInstanceAccess();
		// checkArrayLoadStore();
		// checkInstanceOf();
		// checkCheckCast();
		// checkTableSwitch();
		// checkLookupSwitch();
		// checkGC();
		RTC.avroraBreak();
	}

	// public static void checkGC() {
	// 	System.out.println("1234567 " + RTCTest.check_gc(1234567));
	// }

	// public static void checkLookupSwitch() {
	// 	System.out.println("100 " + RTCTest.check_lookupswitch(0));
	// 	System.out.println("42 " + RTCTest.check_lookupswitch(0x12000000));
	// 	System.out.println("1000000 " + RTCTest.check_lookupswitch(-1000));
	// 	System.out.println("2914 " + RTCTest.check_lookupswitch(0x12345678));
	// }

	// public static void checkTableSwitch() {
	// 	System.out.println("2914 " + RTCTest.check_tableswitch(0x12345677));
	// 	System.out.println("100 " + RTCTest.check_tableswitch(0x12345678));
	// 	System.out.println("42 " + RTCTest.check_tableswitch(0x12345679));
	// 	System.out.println("1000000 " + RTCTest.check_tableswitch(0x1234567a));
	// 	System.out.println("2914 " + RTCTest.check_tableswitch(0x1234567b));
	// }

	// public static void checkCheckCast() {
	// 	RTCTest obj1 = new RTCTest();
	// 	Object obj2 = new Object();

	// 	System.out.println("Testing correct cast:...");
	// 	RTCTest.check_checkcast(obj1);
	// 	System.out.println("Ok.");
	// 	System.out.println("Testing incorrect cast: Now the VM should panic...");
	// 	RTCTest.check_checkcast(obj2);
	// 	System.out.println("Oops, we're still here.");
	// }

	// public static void checkInstanceOf() {
	// 	Object obj1 = new RTCTest();
	// 	Object obj2 = new Object();

	// 	System.out.println("1 " + RTCTest.check_instance_of(obj1));
	// 	System.out.println("0 " + RTCTest.check_instance_of(obj2));
	// }

	// public static void checkArrayLoadStore() {
	// 	byte[] barr = new byte[1];
	// 	barr[0] = (byte)10;
	// 	char[] carr = new char[1];
	// 	carr[0] = (char)100;
	// 	short[] sarr = new short[1];
	// 	sarr[0] = (short)1000;
	// 	RTCTest[] aarr = new RTCTest[1];
	// 	aarr[0] = new RTCTest(10000);
	// 	int[] iarr = new int[1];
	// 	iarr[0] = 100000;

	// 	RTCTest.check_int_array_load_store(barr, carr, sarr, aarr, iarr);

	// 	System.out.println("11 " + barr[0]);
	// 	System.out.println("101 " + (int)carr[0]);
	// 	System.out.println("1001 " + sarr[0]);
	// 	System.out.println("10001 " + aarr[0].instance_int);
	// 	System.out.println("100001 " + iarr[0]);
	// }


	// public static void checkCharByteObjStaticInstanceAccess() {
	// 	RTCTest obj = new RTCTest();

	// 	RTCTest.static_byte1 = -101;
	// 	obj.instance_byte1 = -51;

	// 	RTCTest.static_char1 = 10; // char is just byte in Darjeeling
	// 	obj.instance_char1 = 20;

	// 	RTCTest.static_ref1 = new RTCTest(100000);
	// 	obj.instance_ref1 = new RTCTest(200000);

	// 	RTCTest.check_char_byte_obj_static_instance_access(obj);

	// 	System.out.println("-100 " + RTCTest.static_byte1);
	// 	System.out.println("-50 " + obj.instance_byte1);
	// 	System.out.println("11 " + (int)RTCTest.static_char1);
	// 	System.out.println("21 " + (int)obj.instance_char1);
	// 	System.out.println("100001 " + RTCTest.static_ref1.instance_int);
	// 	System.out.println("200001 " + obj.instance_ref1.instance_int);
	// }

	// public static void checkNewObjectArray() {
	// 	RTCTest[] arr = RTCTest.check_new_object_array();
	// 	System.out.println("2 " + arr.length);
	// 	System.out.println("2 " + arr[0].instance_int);
	// 	System.out.println("7000 " + arr[1].instance_int);
	// }

	// public static void checkNewArray() {
	// 	short[] arr = RTCTest.check_new_array();
	// 	System.out.println("2 " + arr[0]);
	// 	System.out.println("8000 " + arr[1]);
	// 	System.out.println("3 " + arr.length);
	// }

	// public static void checkSomeIntOperations() {
	// 	System.out.println("1127 " +  RTCTest.check_some_int_operations(1000, (short)0, (byte)0, (short)0)); // +127
	// 	System.out.println("872 " +  RTCTest.check_some_int_operations(1000, (short)0, (byte)0, (short)1)); // -128
	// 	System.out.println("2000 " +  RTCTest.check_some_int_operations(2000-0x1234, (short)0, (byte)0, (short)2)); // +0x1234
	// 	System.out.println("-3000 " +  RTCTest.check_some_int_operations(-3000+0x01234, (short)0, (byte)0, (short)3)); // -0x1234
	// 	System.out.println("1042 " +  RTCTest.check_some_int_operations(1000, (short)0, (byte)0, (short)4)); // +42
	// 	System.out.println("42 " +  RTCTest.check_some_int_operations(0x1234562a, (short)0, (byte)0, (short)5)); // cast to byte
	// 	System.out.println("-128 " +  RTCTest.check_some_int_operations(0x80, (short)0, (byte)0, (short)5)); // cast to byte, interpreting 0x80 as a byte makes it negative
	// }

	// public static void checkObjTests() {
	// 	RTCTest obj1 = new RTCTest();
	// 	RTCTest obj2 = new RTCTest();
	// 	RTCTest obj3 = null;

	// 	System.out.println("obj1==obj2 0 " + RTCTest.check_obj_tests(obj1, obj2, (short)0));
	// 	System.out.println("obj1==obj1 1 " + RTCTest.check_obj_tests(obj1, obj1, (short)0));
	// 	System.out.println("null==null 1 " + RTCTest.check_obj_tests(null, null, (short)0));
	// 	System.out.println("obj1!=obj2 1 " + RTCTest.check_obj_tests(obj1, obj2, (short)1));
	// 	System.out.println("obj1!=obj1 0 " + RTCTest.check_obj_tests(obj1, obj1, (short)1));
	// 	System.out.println("null!=null 0 " + RTCTest.check_obj_tests(null, null, (short)1));
	// 	System.out.println("ifnull obj1 0 " + RTCTest.check_obj_tests(obj1, null, (short)2));
	// 	System.out.println("ifnull null 1 " + RTCTest.check_obj_tests(null, null, (short)2));
	// 	System.out.println("ifnull obj3(null) 1 " + RTCTest.check_obj_tests(obj3, null, (short)2));
	// 	System.out.println("ifnonnull obj1 1 " + RTCTest.check_obj_tests(obj1, null, (short)3));
	// 	System.out.println("ifnonnull null 0 " + RTCTest.check_obj_tests(null, null, (short)3));
	// 	System.out.println("ifnonnull obj3(null) 0 " + RTCTest.check_obj_tests(obj3, null, (short)3));
	// 	System.out.println("obj1==tmpobj(null) 0 " + RTCTest.check_obj_tests(obj1, null, (short)4));
	// 	System.out.println("null==tmpobj(null) 1 " + RTCTest.check_obj_tests(null, null, (short)4));		
	// }

	// public static void checkNew() {
	// 	RTCTest obj = RTCTest.check_new();
	// 	System.out.println("2914 " + obj.instance_int);
	// }

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

	// 	obj2 = new RTCTest();
	// 	RTCTest.test_method_call_5(obj, -321);
	// 	System.out.println("654000 " + obj.instance_int);
	// 	RTCTest.test_method_call_5(obj2, 123456);
	// 	System.out.println("123456 " + obj2.instance_int);

	// 	obj.instance_int = -455;
	// 	RTCTest.test_method_call_6(obj2, obj);
	// 	System.out.println("123001 " + obj2.instance_int);		

	// 	System.out.println("142 " + RTCTest.test_method_call_7(100));
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

	// public static void checkStaticRefVariables()
	// {
	// 	RTCTest.static_ref1 = new RTCTest();
	// 	RTCTest.static_ref1.instance_short1 = 101;
	// 	RTCTest.static_ref2 = new RTCTest();
	// 	RTCTest.static_ref2.instance_short1 = 102;

	// 	System.out.println("RTCTest.static_ref1.instance_short1 = " + RTCTest.static_ref1.instance_short1);
	// 	System.out.println("RTCTest.static_ref2.instance_short1 = " + RTCTest.static_ref2.instance_short1);
	// 	System.out.println("Calling RTCTest.test_static_ref_swap()");
	// 	RTCTest.test_static_ref_swap();
	// 	System.out.println("RTCTest.static_ref1.instance_short1 = " + RTCTest.static_ref1.instance_short1);
	// 	System.out.println("RTCTest.static_ref2.instance_short1 = " + RTCTest.static_ref2.instance_short1);
	// }

	// public static void checkCompareInt0()
	// {
	// 	System.out.println("-1 EQ 0 " + RTCTest.compare_int_0_EQ(-1));
	// 	System.out.println(" 0 EQ 1 " + RTCTest.compare_int_0_EQ(0));
	// 	System.out.println(" 1 EQ 0 " + RTCTest.compare_int_0_EQ(1));

	// 	System.out.println("-1 NE 1 " + RTCTest.compare_int_0_NE(-1));
	// 	System.out.println(" 0 NE 0 " + RTCTest.compare_int_0_NE(0));
	// 	System.out.println(" 1 NE 1 " + RTCTest.compare_int_0_NE(1));

	// 	System.out.println("-1 LT 1 " + RTCTest.compare_int_0_LT(-1));
	// 	System.out.println(" 0 LT 0 " + RTCTest.compare_int_0_LT(0));
	// 	System.out.println(" 1 LT 0 " + RTCTest.compare_int_0_LT(1));

	// 	System.out.println("-1 LE 1 " + RTCTest.compare_int_0_LE(-1));
	// 	System.out.println(" 0 LE 1 " + RTCTest.compare_int_0_LE(0));
	// 	System.out.println(" 1 LE 0 " + RTCTest.compare_int_0_LE(1));

	// 	System.out.println("-1 GT 0 " + RTCTest.compare_int_0_GT(-1));
	// 	System.out.println(" 0 GT 0 " + RTCTest.compare_int_0_GT(0));
	// 	System.out.println(" 1 GT 1 " + RTCTest.compare_int_0_GT(1));

	// 	System.out.println("-1 GE 0 " + RTCTest.compare_int_0_GE(-1));
	// 	System.out.println(" 0 GE 1 " + RTCTest.compare_int_0_GE(0));
	// 	System.out.println(" 1 GE 1 " + RTCTest.compare_int_0_GE(1));
	// }

	// public static void checkCompareShort0()
	// {
	// 	System.out.println("-1 EQ 0 " + RTCTest.compare_short_0_EQ((short)-1));
	// 	System.out.println(" 0 EQ 1 " + RTCTest.compare_short_0_EQ((short)0));
	// 	System.out.println(" 1 EQ 0 " + RTCTest.compare_short_0_EQ((short)1));

	// 	System.out.println("-1 NE 1 " + RTCTest.compare_short_0_NE((short)-1));
	// 	System.out.println(" 0 NE 0 " + RTCTest.compare_short_0_NE((short)0));
	// 	System.out.println(" 1 NE 1 " + RTCTest.compare_short_0_NE((short)1));

	// 	System.out.println("-1 LT 1 " + RTCTest.compare_short_0_LT((short)-1));
	// 	System.out.println(" 0 LT 0 " + RTCTest.compare_short_0_LT((short)0));
	// 	System.out.println(" 1 LT 0 " + RTCTest.compare_short_0_LT((short)1));

	// 	System.out.println("-1 LE 1 " + RTCTest.compare_short_0_LE((short)-1));
	// 	System.out.println(" 0 LE 1 " + RTCTest.compare_short_0_LE((short)0));
	// 	System.out.println(" 1 LE 0 " + RTCTest.compare_short_0_LE((short)1));

	// 	System.out.println("-1 GT 0 " + RTCTest.compare_short_0_GT((short)-1));
	// 	System.out.println(" 0 GT 0 " + RTCTest.compare_short_0_GT((short)0));
	// 	System.out.println(" 1 GT 1 " + RTCTest.compare_short_0_GT((short)1));

	// 	System.out.println("-1 GE 0 " + RTCTest.compare_short_0_GE((short)-1));
	// 	System.out.println(" 0 GE 1 " + RTCTest.compare_short_0_GE((short)0));
	// 	System.out.println(" 1 GE 1 " + RTCTest.compare_short_0_GE((short)1));
	// }

	// public static void checkCompareInt()
	// {
	// 	System.out.println("-1 EQ 0 " + RTCTest.compare_int_EQ(-100001, -100000));
	// 	System.out.println(" 0 EQ 1 " + RTCTest.compare_int_EQ(-100000, -100000));
	// 	System.out.println(" 1 EQ 0 " + RTCTest.compare_int_EQ(-99999, -100000));

	// 	System.out.println("-1 NE 1 " + RTCTest.compare_int_NE(-100001, -100000));
	// 	System.out.println(" 0 NE 0 " + RTCTest.compare_int_NE(-100000, -100000));
	// 	System.out.println(" 1 NE 1 " + RTCTest.compare_int_NE(-99999, -100000));

	// 	System.out.println("-1 LT 1 " + RTCTest.compare_int_LT(-100001, -100000));
	// 	System.out.println(" 0 LT 0 " + RTCTest.compare_int_LT(-100000, -100000));
	// 	System.out.println(" 1 LT 0 " + RTCTest.compare_int_LT(-99999, -100000));

	// 	System.out.println("-1 LE 1 " + RTCTest.compare_int_LE(-100001, -100000));
	// 	System.out.println(" 0 LE 1 " + RTCTest.compare_int_LE(-100000, -100000));
	// 	System.out.println(" 1 LE 0 " + RTCTest.compare_int_LE(-99999, -100000));

	// 	System.out.println("-1 GT 0 " + RTCTest.compare_int_GT(-100001, -100000));
	// 	System.out.println(" 0 GT 0 " + RTCTest.compare_int_GT(-100000, -100000));
	// 	System.out.println(" 1 GT 1 " + RTCTest.compare_int_GT(-99999, -100000));

	// 	System.out.println("-1 GE 0 " + RTCTest.compare_int_GE(-100001, -100000));
	// 	System.out.println(" 0 GE 1 " + RTCTest.compare_int_GE(-100000, -100000));
	// 	System.out.println(" 1 GE 1 " + RTCTest.compare_int_GE(-99999, -100000));
	// }

	// public static void checkCompareShort()
	// {
	// 	System.out.println("-1 EQ 0 " + RTCTest.compare_short_EQ((short)-101, (short)-100));
	// 	System.out.println(" 0 EQ 1 " + RTCTest.compare_short_EQ((short)-100, (short)-100));
	// 	System.out.println(" 1 EQ 0 " + RTCTest.compare_short_EQ((short)-99, (short)-100));

	// 	System.out.println("-1 NE 1 " + RTCTest.compare_short_NE((short)-101, (short)-100));
	// 	System.out.println(" 0 NE 0 " + RTCTest.compare_short_NE((short)-100, (short)-100));
	// 	System.out.println(" 1 NE 1 " + RTCTest.compare_short_NE((short)-99, (short)-100));

	// 	System.out.println("-1 LT 1 " + RTCTest.compare_short_LT((short)-101, (short)-100));
	// 	System.out.println(" 0 LT 0 " + RTCTest.compare_short_LT((short)-100, (short)-100));
	// 	System.out.println(" 1 LT 0 " + RTCTest.compare_short_LT((short)-99, (short)-100));

	// 	System.out.println("-1 LE 1 " + RTCTest.compare_short_LE((short)-101, (short)-100));
	// 	System.out.println(" 0 LE 1 " + RTCTest.compare_short_LE((short)-100, (short)-100));
	// 	System.out.println(" 1 LE 0 " + RTCTest.compare_short_LE((short)-99, (short)-100));

	// 	System.out.println("-1 GT 0 " + RTCTest.compare_short_GT((short)-101, (short)-100));
	// 	System.out.println(" 0 GT 0 " + RTCTest.compare_short_GT((short)-100, (short)-100));
	// 	System.out.println(" 1 GT 1 " + RTCTest.compare_short_GT((short)-99, (short)-100));

	// 	System.out.println("-1 GE 0 " + RTCTest.compare_short_GE((short)-101, (short)-100));
	// 	System.out.println(" 0 GE 1 " + RTCTest.compare_short_GE((short)-100, (short)-100));
	// 	System.out.println(" 1 GE 1 " + RTCTest.compare_short_GE((short)-99, (short)-100));
	// }

	public static void checkReturnValues()
	{
		System.out.println("Boolean true " + RTCTest.testReturnBoolean());
		System.out.println("Byte -42 " + RTCTest.testReturnByte());
		System.out.println("Short -4200 " + RTCTest.testReturnShort());
		System.out.println("Int -420000 " + RTCTest.testReturnInt());
		System.out.println("Char x " + RTCTest.testReturnChar());
		System.out.println("Ref -42 " + RTCTest.testReturnRef().instance_int);
	}
}
