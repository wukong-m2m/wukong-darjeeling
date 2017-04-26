package javax.rtcbench;


import javax.rtc.RTC;
import javax.rtc.Lightweight;

//// UNITTEST

public class RTCBenchmark {
    public static String name = "TEST LIGHTWEIGHT METHOD";
    public static native void test_native();

    public static boolean test_java() {
        return rtcbenchmark_measure_java_performance();
    }

    public static boolean rtcbenchmark_measure_java_performance() {
        boolean success = true;

        short i,j=3;
        int k=12;
        for (i = 0; i<10; i++) {
            // This loop should pin 4 pairs, but the methods inside shouldn't use all.
            // Check if INVOKELIGHT saves the right registers (only those that are used
            // by the method it calls)
            System.out.println("round " + i);
            System.out.println("test to pin more regs " + (i*k)+j);

            success = success && testLightweightJavaMethod(4200043, (byte) -1) == 4200042;
            success = success && testLightweightJavaMethodWithLoop(42, (byte) 5) == 4200000;
            success = success && testSIMUL((short)1000, (short)4200) == 4200000;
            success = success && testNestedLightweight((short)1000, (short)4200) == 4200000;

            success = success && testISWAP((short) 42, 4200000) == 4200042;
            success = success && testLOAD_STORE(4200000, (short) 42) == 4200042;

            success = success && isOddShort((short)2) == false;
            success = success && isOddShort((short)-3) == true;
            success = success && isOddInt(3) == true;
            success = success && isOddInt(-2) == false;

            success = success && isNull(null) == true;
            success = success && isNull("null") == false;
            success = success && isNull(new Object()) == false;

            // This should create a high stack depth. Check to see if maxStack of this method
            // is correctly increased to reserve space for the lightweight method's stack.
            // (actually for ints it shouldn't matter. add another test for refs later.)
            success = success && timesTenTestHighStackShort((short)123) == 1230;
            success = success && timesTenTestHighStackRef(null) == 42;
            success = success && timesTenTestHighStackRef("null") == 42;
            success = success && timesTenTestHighStackRef(new Object()) == 42;

            success = success && (CoreState.ee_isdigit_lightweight((byte)'/') == false); // 0x2f
            success = success && (CoreState.ee_isdigit_lightweight((byte)'0') == true);  // 0x30
            success = success && (CoreState.ee_isdigit_lightweight((byte)'9') == true);  // 0x39
            success = success && (CoreState.ee_isdigit_lightweight((byte)':') == false); // 0x3a

            success = success && ((short)CoreUtil.crc(0xF2345678,(short)0xFBCD,(short)1) == (short)0xb73a);
            success = success && ((short)CoreUtil.crc(0xF2345678,(short)0xFBCD,(short)2) == (short)0x2db7);
            success = success && ((short)CoreUtil.crc(0xF2345678,(short)0xFBCD,(short)4) == (short)0xa820);

            if (success) { System.out.println("ok"); } else { System.out.println("kaput"); }
        }

        success = success && i==10;
        System.out.println("total number of rounds: " + i);

        return success;
    }

    public static boolean isOddShort_notlight(short x) { return (x & (short)1) == (short)1; }
    public static boolean isOddInt_notlight(int x) { return (x & 1) == 1; }
    public static boolean isNull_notlight(Object x) { return x == null; }
    public static int addXZand1ifYnotnull_notlight(int x, Object y, short z) { return x + (y == null ? 0 : 1) + z; }
    public static short timesTenTestHighStackShort_notlight(short x) { return (short)((short)x+
                                                                     (short)((short)x+
                                                                     (short)((short)x+
                                                                     (short)((short)x+
                                                                     (short)((short)x+
                                                                     (short)((short)x+
                                                                     (short)((short)x+
                                                                     (short)((short)x+
                                                                     (short)((short)x+
                                                                     (short)((short)x)))))))))); }

    // Native lightweight methods have their implementation hardcoded in the infuser.
    // (deliberately put at the bottom of the class to check if the infuser will move them to the start of the infusion)    
    @Lightweight
    public static native int testISWAP(short a, int b);
    @Lightweight
    public static native int testLOAD_STORE(int a, short b);
    @Lightweight
    public static native boolean isOddShort(short x);
    @Lightweight
    public static native boolean isOddInt(int x);
    @Lightweight
    public static native boolean isNull(Object x);
    @Lightweight
    public static native short timesTenTestHighStackShort(short x);
    @Lightweight
    public static native short timesTenTestHighStackRef(Object x);

    @Lightweight
    public static int testLightweightJavaMethod(int a, byte b) {
        return a+b;
    }
    @Lightweight
    public static int testLightweightJavaMethodWithLoop(int a, byte b) {
        // Loop should be the first in the method to test if the BRTARGET isn't placed
        // before the extra STORE instructions the infuser will prepend to the method.
        while (b>0) {
            int a_copy = a;
            for (int i=0; i<9; i++) {
                a += a_copy;
            }
            b--;
        }
        return a;
    }
    @Lightweight
    public static int testSIMUL(short a, short b) {
        return a*b;
    }
    @Lightweight
    public static int testNestedLightweight3(short a, short b) {
        return a*b;
    }
    @Lightweight
    public static int testNestedLightweight2(short a, short b) {
        return testNestedLightweight3(a, b);
    }
    @Lightweight
    public static int testNestedLightweight(short a, short b) {
        return testNestedLightweight2(a, b);
    }
}
