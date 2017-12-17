package javax.rtcbench;

import javax.rtc.RTC;

public class RTCBenchmark {
    public static String name = "TINY TEST";
    public static native void test_native();

    // // EMPTY
    // public static boolean test_java() {
    //     rtcbenchmark_measure_java_performance();
    //     return true;
    // }
    // public static void rtcbenchmark_measure_java_performance() {
    // }

    // Example of lack of optimisations in javac: this will calculate b*c on each iteration
    public static int test_loop(int a, int b, int c) {
        while (a < b*c) {
            a++;
        }
        return a;
    }

    public static boolean isEven(int a) {
        return a%2==0;
    }

    public static boolean isEvenDontInline(int a) {
        return a%2==0;
    }

    public static int test_no_inlining(int a) {
        while (isEvenDontInline(a)) {
            a++;
        }
        return a;
    }

    public static int test_pg_inlining(int a) {
        while (isEven(a)) {
            a++;
        }
        return a;
    }

    public static int test_manual_inlining(int a) {
        while (a%2==0) {
            a++;
        }
        return a;
    }

    // CHECKCAST
    public static boolean test_java() {
        return rtcbenchmark_measure_java_performance();
    }
    public static boolean rtcbenchmark_measure_java_performance() {
        Object obj = new Object();
        Exception ex;
        try {
            throw new Exception();
            // ex = (Exception)obj;
        } catch (Exception ex2) {
            return true;
        }
    }

}
