package javax.rtcbench;

import javax.rtc.RTC;

//// UNITTEST

public class RTCBenchmark {
    public static String name = "TEST BIG OFFSETS";
    public static native void test_native();

    public static boolean test_java() {
        return rtcbenchmark_measure_java_performance();
    }

    public static int test_static_int1;
    public static int test_static_int2;
    public static int test_static_int3;
    public static int test_static_int4;
    public static int test_static_int5;
    public static int test_static_int6;
    public static int test_static_int7;
    public static int test_static_int8;
    public static int test_static_int9;
    public static int test_static_int10;
    public static int test_static_int11;
    public static int test_static_int12;
    public static int test_static_int13;
    public static int test_static_int14;
    public static int test_static_int15;
    public static int test_static_int16;
    public static int test_static_int17;

    public static testClass test_static_obj1;
    public static testClass test_static_obj2;
    public static testClass test_static_obj3;
    public static testClass test_static_obj4;
    public static testClass test_static_obj5;
    public static testClass test_static_obj6;
    public static testClass test_static_obj7;
    public static testClass test_static_obj8;
    public static testClass test_static_obj9;
    public static testClass test_static_obj10;
    public static testClass test_static_obj11;
    public static testClass test_static_obj12;
    public static testClass test_static_obj13;
    public static testClass test_static_obj14;
    public static testClass test_static_obj15;
    public static testClass test_static_obj16;
    public static testClass test_static_obj17;
    public static testClass test_static_obj18;
    public static testClass test_static_obj19;
    public static testClass test_static_obj20;
    public static testClass test_static_obj21;
    public static testClass test_static_obj22;
    public static testClass test_static_obj23;
    public static testClass test_static_obj24;
    public static testClass test_static_obj25;
    public static testClass test_static_obj26;
    public static testClass test_static_obj27;
    public static testClass test_static_obj28;
    public static testClass test_static_obj29;
    public static testClass test_static_obj30;
    public static testClass test_static_obj31;
    public static testClass test_static_obj32;
    public static testClass test_static_obj33;

    public static class testClass {
        public short test_short1;
        public int test_int1;
        public int test_int2;
        public int test_int3;
        public int test_int4;
        public int test_int5;
        public int test_int6;
        public int test_int7;
        public int test_int8;
        public int test_int9;
        public int test_int10;
        public int test_int11;
        public int test_int12;
        public int test_int13;
        public int test_int14;
        public int test_int15;
        public int test_int16;
        public int test_int17;
        public short test_short17;

        public testClass test_obj1;
        public testClass test_obj2;
        public testClass test_obj3;
        public testClass test_obj4;
        public testClass test_obj5;
        public testClass test_obj6;
        public testClass test_obj7;
        public testClass test_obj8;
        public testClass test_obj9;
        public testClass test_obj10;
        public testClass test_obj11;
        public testClass test_obj12;
        public testClass test_obj13;
        public testClass test_obj14;
        public testClass test_obj15;
        public testClass test_obj16;
        public testClass test_obj17;
        public testClass test_obj18;
        public testClass test_obj19;
        public testClass test_obj20;
        public testClass test_obj21;
        public testClass test_obj22;
        public testClass test_obj23;
        public testClass test_obj24;
        public testClass test_obj25;
        public testClass test_obj26;
        public testClass test_obj27;
        public testClass test_obj28;
        public testClass test_obj29;
        public testClass test_obj30;
        public testClass test_obj31;
        public testClass test_obj32;
        public testClass test_obj33;
    }

    public static void setStuff(testClass c) {
        c.test_int1 = 41;
        c.test_int17 = 42;
        c.test_short1 = 43;
        c.test_short17 = 44;
        c.test_obj1 = c;
        c.test_obj33 = c;
        test_static_int1 = 46;
        test_static_int17 = 47;
    }

    public static void incStuff(testClass c) {
        short test_short1 = 0;
        int test_int1 = 0;
        int test_int2 = 0;
        int test_int3 = 0;
        int test_int4 = 0;
        int test_int5 = 0;
        int test_int6 = 0;
        int test_int7 = 0;
        int test_int8 = 0;
        int test_int9 = 0;
        int test_int10 = 0;
        int test_int11 = 0;
        int test_int12 = 0;
        int test_int13 = 0;
        int test_int14 = 0;
        int test_int15 = 0;
        int test_int16 = 0;
        int test_int17 = 0;
        short test_short17 = 0;


        test_int1 = c.test_int1;
        test_short1 = c.test_short1;

        test_int2 = test_int1;
        test_int3 = test_int2;
        test_int4 = test_int3;
        test_int5 = test_int4;
        test_int6 = test_int5;
        test_int7 = test_int6;
        test_int8 = test_int7;
        test_int9 = test_int8;
        test_int10 =test_int9;
        test_int11 = test_int10;
        test_int12 = test_int11;
        test_int13 = test_int12;
        test_int14 = test_int13;
        test_int15 = test_int14;
        test_int16 = test_int15;

        test_int17 = c.test_int17;
        test_short17 = c.test_short17;

        test_int1++;
        test_int17++;
        test_short1++;
        test_short17++;

        c.test_int1 = test_int1;
        c.test_int17 = test_int17;
        c.test_short1 = test_short1;
        c.test_short17 = test_short17;
    }

    public static boolean rtcbenchmark_measure_java_performance() {
        boolean success = true;

        testClass c = new testClass();

        setStuff(c);
        incStuff(c);

        success = success && c.test_int1 == 42;
        success = success && c.test_int17 == 43;
        success = success && c.test_short1 == 44;
        success = success && c.test_short17 == 45;
        success = success && c.test_obj1.test_int1 == 42;
        success = success && c.test_obj17 == null;
        success = success && c.test_obj33.test_int1 == 42;
        success = success && test_static_int1 == 46;
        success = success && test_static_int17 == 47;

        return success;
    }
}
