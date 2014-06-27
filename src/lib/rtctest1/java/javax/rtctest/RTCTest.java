package javax.rtctest;

// This won't be RTC compiled. Used to test access to static variables in other infusions, and calls from RTC to JVM code.
import javax.rtctest2.RTCTest2;

public class RTCTest implements IRTCTest {
	private static short static_short1;
	private static short static_short2;
	private static short static_short3;
	private static int static_int1;
	private static int static_int2;
	public static RTCTest static_ref1;
	public static RTCTest static_ref2;
	public static byte static_byte1;
	public static char static_char1;

	public short instance_short1;
	public int instance_int;
	public short instance_short2;
	public byte instance_byte1;
	public char instance_char1;
	private RTCTest instance_ref0;
	public RTCTest instance_ref1;

	public RTCTest() {}
	public RTCTest(int init_int_value) {
		this.instance_int = init_int_value;
	}

	// public static int check_gc(int a) {
	// 	Runtime.getRuntime().gc();
	// 	new RTCTest(); // allocate some memory and immediately release the object again
	// 	return check_gc2(a); // there will be empty space below check_gc2's stack frame, so triggering GC will cause it to shift down
	// }
	// public static int check_gc2(int a) {
	// 	Runtime.getRuntime().gc();
	// 	for (int i=0; i<5; i++)
	// 		new RTCTest(); // allocate some memory and immediately release the object again
	// 	return a+1;
	// }

	// public static int check_lookupswitch(int a) {
	// 	switch(a) {
	// 		case 0:
	// 			return 100;
	// 		case 0x12000000:
	// 			return 42;
	// 		case -1000:
	// 			return 1000000;
	// 		default:
	// 			return 2914;
	// 	}
	// }

	// public static int check_tableswitch(int a) {
	// 	switch(a) {
	// 		case 0x12345678:
	// 			return 100;
	// 		case 0x12345679:
	// 			return 42;
	// 		case 0x1234567a:
	// 			return 1000000;
	// 		default:
	// 			return 2914;
	// 	}
	// }

	// public static RTCTest check_checkcast(Object obj) {
	// 	return (RTCTest)obj; // Should just return obj if it's an RTCTest, or panic if it's not (since we won't implement exceptions)
	// }

	// public static int check_instance_of(Object obj) {
	// 	if (obj instanceof RTCTest)
	// 		return 1;
	// 	else
	// 		return 0;
	// }

	// public static void check_int_array_load_store(byte[] barr, char[] carr, short[] sarr, RTCTest[] aarr, int[] iarr) {
	// 	barr[0]++;
	// 	carr[0]++;
	// 	sarr[0]++;
	// 	aarr[0] = new RTCTest(aarr[0].instance_int+1);
	// 	iarr[0]++;
	// }

	// public static void check_char_byte_obj_static_instance_access(RTCTest obj) {
	// 	// bytes
	// 	static_byte1++;
	// 	obj.instance_byte1++;

	// 	// chars
	// 	static_char1++;
	// 	obj.instance_char1++;

	// 	// refs
	// 	int x = static_ref1.instance_int;
	// 	RTCTest new_instance = new RTCTest(x+1);
	// 	static_ref1 = new_instance;

	// 	x = obj.instance_ref1.instance_int;
	// 	new_instance = new RTCTest(x+1);
	// 	obj.instance_ref1 = new_instance;
	// }

	// public static RTCTest[] check_new_object_array() {
	// 	RTCTest[] arr = new RTCTest[2];
	// 	arr[0] = new RTCTest(2);
	// 	arr[1] = new RTCTest(6000);
	// 	arr[1].instance_int += 1000; // test AALOAD as well
	// 	return arr;
	// }

	// public static short[] check_new_array() {
	// 	short[] arr = new short[3];
	// 	arr[0] = 2;
	// 	arr[1] = 8000;
	// 	return arr;
	// }

	// public static int check_some_int_operations(int a, short b, byte c, short test) {
	// 	if (test == 0) {
	// 		// test IINC +127
	// 		a += 127;
	// 		return a;
	// 	}
	// 	if (test == 1) {
	// 		// test IINC -128
	// 		a -= 128;
	// 		return a;
	// 	}
	// 	if (test == 2) {
	// 		// test IINC_W +128
	// 		a += 0x1234;
	// 		return a;
	// 	}
	// 	if (test == 3) {
	// 		// test IINC_W -129
	// 		a -= 0x1234;
	// 		return a;
	// 	}
	// 	if (test == 4) {
	// 		// test BIPUSH
	// 		return a+42;
	// 	}
	// 	if (test == 5) {
	// 		// test S2B, S2I (S2I for return value. should sign extend.)
	// 		return (byte)a;
	// 	}

	// 	return 0;
	// }


	// public static int check_obj_tests(RTCTest obj1, RTCTest obj2, short test) {
	// 	if (test == 0)
	// 		return obj1 == obj2 ? 1 : 0;
	// 	if (test == 1)
	// 		return obj1 != obj2 ? 1 : 0;
	// 	if (test == 2)
	// 		return obj1 == null ? 1 : 0;
	// 	if (test == 3)
	// 		return obj1 != null ? 1 : 0;
	// 	if (test == 4) {
	// 		RTCTest tmpobj = null;
	// 		return obj1 == tmpobj ? 1 : 0;
	// 	}
	// 	return 2;
	// }

	// public static RTCTest check_new() {
	// 	RTCTest obj = new RTCTest(2914);
	// 	return obj;
	// }

	// public static void check_calling_println() {
	// 	System.out.println("Avenues all lined with trees.");
	// }

	// public static RTCTest test_return_object(RTCTest obj) {
	// 	obj.instance_int++;
	// 	return obj;
	// }

	// public static int test_method_call_1(int a, short b, short c, int d) {
	// 	return test_method_call_1b(a, b, c, d);
	// }
	// public static int test_method_call_1b(int a, short b, short c, int d) {
	// 	// Just test a bunch of ints.
	// 	// This should be OK even without reserving space on the int stack since we pass more than will be returned, thus ensuring enough stack space for the return value.
	// 	return (a - b) + 1 + (d - c);
	// }
	// public static int test_method_call_2(int a, short b, RTCTest obj, short c, int d) {
	// 	return test_method_call_2b(a, b, obj, c, d);
	// }
	// public static int test_method_call_2b(int a, short b, RTCTest obj, short c, int d) {
	// 	// Add passing an object
	// 	return (a - b) + 1 + (d - c) + obj.instance_short1;
	// }
	// public static int test_method_call_3(RTCTest obj) {
	// 	return test_method_call_3b(obj);
	// }
	// public static int test_method_call_3b(RTCTest obj) {
	// 	// This method returns more than it gets passed on the int stack
	// 	// This will crash the VM if we don't reserve extra space on the system stack
	// 	return 100000+obj.instance_short1;
	// }
	// public static RTCTest test_method_call_4(RTCTest obj) {
	// 	return test_method_call_4b(obj);
	// }
	// public static RTCTest test_method_call_4b(RTCTest obj) {
	// 	// Test returning objects from rtc to rtc
	// 	obj.instance_int++;
	// 	return obj;
	// }
	// public static int test_method_call_5(RTCTest obj, int a) {
	// 	return obj.test_method_call_5b(a);
	// }
	// public int test_method_call_5b(int a) {
	// 	this.instance_int += a;
	// 	return this.instance_int;
	// }
	// public static int test_method_call_6(IRTCTest obj, RTCTest obj2) {
	// 	return obj.test_method_call_6b(obj2);
	// }
	// Can't comment this because we wouldn't be implemening IRTCTest anymore.
	public int test_method_call_6b(RTCTest obj) {
		this.instance_int += obj.instance_int;
		return this.instance_int;
	}
	// public static int test_method_call_7(int a) {
	// 	return RTCTest2.test_method_call_7b(a);
	// }

	// public static short test_method_call(short a, RTCTest obj) {
	// 	// return test_method_call2(a, (short)42, obj);
	// 	return (short)(test_method_call2(a, (short)42, obj) % (short)100);
	// }

	// public static short test_method_call2(short a, short b, RTCTest obj) {
	// 	// This should be OK even without reserving space on the int stack since we pass more than will be returned.
	// 	return (short)(a + b + obj.instance_short1);
	// }

	// public static int test_method_call2(short a, RTCTest obj) {
	// 	// Will return an int (4 bytes), but only consumer a short (2 bytes)
	// 	// This means we need to reserve 2 bytes on the real/int stack.
	// 	return a + 42 + obj.instance_short1;
	// }

	// public static void test_bubblesort(short[] numbers) {
	// 	short NUMNUMBERS = (short)numbers.length;
	// 	for (short i=0; i<(short)NUMNUMBERS; i++) {
	// 		for (short j=0; j<((short)(NUMNUMBERS-i-1)); j++) {
	// 			if (numbers[j]>numbers[j+1]) {
	// 				short temp = numbers[j];
	// 				numbers[j] = numbers[j+1];
	// 				numbers[((short)(j+1))] = temp;
	// 			}
	// 		}
	// 	}
	// // }

	// public static short test_short_ops(short x, short y, short op) {
	// 	if (op==0) return (short) ( -x);
	// 	if (op==1) return (short) (x + y);
	// 	if (op==2) return (short) (x - y);
	// 	if (op==3) return (short) (x * y);
	// 	if (op==4) return (short) (x / y);
	// 	if (op==5) return (short) (x % y);
	// 	if (op==6) return (short) (x & y);
	// 	if (op==7) return (short) (x | y);
	// 	if (op==8) return (short) (x ^ y);
	// 	if (op==9) return (short) (x + (-1));
	// 	if (op==10) return (short) (x + 42);
	// 	if (op==11) return (short) (x << y);
	// 	if (op==12) return (short) (x >> y);
	// 	if (op==13) return (short) (x >>> y);
	// 	return (short) (-42);
	// }

	// public static int test_int_ops(int x, int y, short op) {
	// 	if (op==0) return  -x;
	// 	if (op==1) return x + y;
	// 	if (op==2) return x - y;
	// 	if (op==3) return x * y;
	// 	if (op==4) return x / y;
	// 	if (op==5) return x % y;
	// 	if (op==6) return x & y;
	// 	if (op==7) return x | y;
	// 	if (op==8) return x ^ y;
	// 	if (op==9) return x + (-1);
	// 	if (op==10) return x + 42;
	// 	if (op==11) return x << y;
	// 	if (op==12) return x >> y;
	// 	if (op==13) return x >>> y;
	// 	return -42;
	// }

	// public static short test_fib(short x) {
	// 	if (x == 0)
	// 		return 0;
	// 	if (x == 1)
	// 		return 1;
	// 	short previous = 0;
	// 	short current = 1;
	// 	while (x != 1) {
	// 		short new_current = (short)(previous + current);
	// 		previous = current;
	// 		current = new_current;
	// 		x--;
	// 	}
	// 	return current;
	// }

	// public short test_instance_short1(short x) {
	// 	this.instance_short1 += x;
	// 	return this.instance_short1;
	// }

	// public short test_instance_short2(short x) {
	// 	this.instance_short2 += x;
	// 	return this.instance_short2;
	// }

	// public static short test_static_short(short x) {
	// 	static_short2 += x;
	// 	return static_short2;
	// }
	// public static short test_static_short_in_other_infusion_1(short x) {
	// 	RTCTest2.rtc_test_short1 += x;
	// 	return RTCTest2.rtc_test_short1;
	// }
	// public static short test_static_short_in_other_infusion_2(short x) {
	// 	RTCTest2.rtc_test_short2 += x;
	// 	return RTCTest2.rtc_test_short2;
	// }

	// public static int test_static_int(int x) {
	// 	static_int2 += x;
	// 	return static_int2;
	// }

	// public static int test_static_int_in_other_infusion_1(int x) {
	// 	RTCTest2.rtc_test_int1 += x;
	// 	return RTCTest2.rtc_test_int1;
	// }

	// public static int test_static_int_in_other_infusion_2(int x) {
	// 	RTCTest2.rtc_test_int2 += x;
	// 	return RTCTest2.rtc_test_int2;
	// }

	// public static void test_static_ref_swap() {
	// 	RTCTest obj = RTCTest.static_ref1;
	// 	RTCTest.static_ref1 = RTCTest.static_ref2;
	// 	RTCTest.static_ref2 = obj;
	// }

	// public static int compare_int_0_EQ(int x) {
	// 	if (x == 0)
	// 		return 1;
	// 	else
	// 		return 0;
	// }
	// public static int compare_int_0_NE(int x) {
	// 	if (x != 0)
	// 		return 1;
	// 	else
	// 		return 0;
	// }
	// public static int compare_int_0_LT(int x) {
	// 	if (x < 0)
	// 		return 1;
	// 	else
	// 		return 0;
	// }
	// public static int compare_int_0_LE(int x) {
	// 	if (x <= 0)
	// 		return 1;
	// 	else
	// 		return 0;
	// }
	// public static int compare_int_0_GT(int x) {
	// 	if (x > 0)
	// 		return 1;
	// 	else
	// 		return 0;
	// }
	// public static int compare_int_0_GE(int x) {
	// 	if (x >= 0)
	// 		return 1;
	// 	else
	// 		return 0;
	// }

	// public static short compare_short_0_EQ(short x) {
	// 	if (x == 0)
	// 		return (short)1;
	// 	else
	// 		return (short)0;
	// }
	// public static short compare_short_0_NE(short x) {
	// 	if (x != 0)
	// 		return (short)1;
	// 	else
	// 		return (short)0;
	// }
	// public static short compare_short_0_LT(short x) {
	// 	if (x < 0)
	// 		return (short)1;
	// 	else
	// 		return (short)0;
	// }
	// public static short compare_short_0_LE(short x) {
	// 	if (x <= 0)
	// 		return (short)1;
	// 	else
	// 		return (short)0;
	// }
	// public static short compare_short_0_GT(short x) {
	// 	if (x > 0)
	// 		return (short)1;
	// 	else
	// 		return (short)0;
	// }
	// public static short compare_short_0_GE(short x) {
	// 	if (x >= 0)
	// 		return (short)1;
	// 	else
	// 		return (short)0;
	// }

	// public static int compare_int_EQ(int x, int y) {
	// 	if (x == y)
	// 		return 1;
	// 	else
	// 		return 0;
	// }
	// public static int compare_int_NE(int x, int y) {
	// 	if (x != y)
	// 		return 1;
	// 	else
	// 		return 0;
	// }
	// public static int compare_int_LT(int x, int y) {
	// 	if (x < y)
	// 		return 1;
	// 	else
	// 		return 0;
	// }
	// public static int compare_int_LE(int x, int y) {
	// 	if (x <= y)
	// 		return 1;
	// 	else
	// 		return 0;
	// }
	// public static int compare_int_GT(int x, int y) {
	// 	if (x > y)
	// 		return 1;
	// 	else
	// 		return 0;
	// }
	// public static int compare_int_GE(int x, int y) {
	// 	if (x >= y)
	// 		return 1;
	// 	else
	// 		return 0;
	// }

	// public static short compare_short_EQ(short x, short y) {
	// 	if (x == y)
	// 		return (short)1;
	// 	else
	// 		return (short)0;
	// }

	// public static short compare_short_NE(short x, short y) {
	// 	if (x != y)
	// 		return (short)1;
	// 	else
	// 		return (short)0;
	// }

	// public static short compare_short_LT(short x, short y) {
	// 	if (x < y)
	// 		return (short)1;
	// 	else
	// 		return (short)0;
	// }

	// public static short compare_short_LE(short x, short y) {
	// 	if (x <= y)
	// 		return (short)1;
	// 	else
	// 		return (short)0;
	// }

	// public static short compare_short_GT(short x, short y) {
	// 	if (x > y)
	// 		return (short)1;
	// 	else
	// 		return (short)0;
	// }

	// public static short compare_short_GE(short x, short y) {
	// 	if (x >= y)
	// 		return (short)1;
	// 	else
	// 		return (short)0;
	// }

	// public static native short GetFortyThree();

	// public static short Add(short a, short b, short c, short d, short e) {
	// 	return (short)(a+b+c+d+e);
	// }
}
