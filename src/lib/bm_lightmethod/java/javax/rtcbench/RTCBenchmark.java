package javax.rtcbench;

import javax.darjeeling.Stopwatch;

import javax.rtc.RTC;

//// UNITTEST

public class RTCBenchmark {
    public static String name = "TEST LIGHTWEIGHT METHOD";
    public static native void test_native();

    public static boolean test_java() {
        return rtcbenchmark_measure_java_performance();
    }
    
    public static native boolean isOddInt(int x);
    public static native boolean isOddShort(short x);
    // public static native boolean isNull(Object x);
    // public static native int addXZand1ifYnotnull(int x, Object y, short z);
    public static native short timesTenTestHighStackShort(short x);
    // public static native int timesTenTestHighStackInt(int x);

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

            success = success && isOddShort((short)2) == false;
            success = success && isOddShort((short)-3) == true;
            success = success && isOddInt(3) == true;
            success = success && isOddInt(-2) == false;
            // success = success && isNull(null) == true;
            // success = success && isNull("null") == false;

            // success = success && addXZand1ifYnotnull(500000, "null", (short)5) == 500006;
            // success = success && addXZand1ifYnotnull(500000, null, (short)5) == 500005;

            // This should create a high stack depth. Check to see if maxStack of this method
            // is correctly increased to reserve space for the lightweight method's stack.
            // (actually for ints it shouldn't matter. add another test for refs later.)
            success = success && timesTenTestHighStackShort((short)123) == 1230;
            // success = success && timesTenTestHighStackInt(123456) == 1234560;

            if (success) {
                System.out.println("ok");
            } else {                
                System.out.println("kaput");
            }
        }

        success = success && i==10;
        System.out.println("total number of rounds: " + i);

        return success;
    }

    // public static boolean isOddShort(short x) { return (x & (short)1) == (short)1; }
    // public static boolean isOddInt(int x) { return (x & 1) == 1; }
    // public static boolean isNull(Object x) { return x == null; }
    // public static int addXZand1ifYnotnull(int x, Object y, short z) { return x + (y == null ? 0 : 1) + z; }
    // public static short timesTenTestHighStackShort(short x) { return (short)((short)x+
    //                                                                  (short)((short)x+
    //                                                                  (short)((short)x+
    //                                                                  (short)((short)x+
    //                                                                  (short)((short)x+
    //                                                                  (short)((short)x+
    //                                                                  (short)((short)x+
    //                                                                  (short)((short)x+
    //                                                                  (short)((short)x+
    //                                                                  (short)((short)x)))))))))); }
    // public static int timesTenTestHighStackInt(int x) { return x+x+x+x+x+x+x+x+x+x; }
}
