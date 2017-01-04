package javax.rtcbench;

import javax.darjeeling.Stopwatch;

public class RTCBenchmark {
    public static String name = "TINY TEST";
    public static native void test_native();

    // // EMPTY
    // public static boolean test_java() {
    //     rtcbenchmark_measure_java_performance();
    //     return true;
    // }
    // public static void rtcbenchmark_measure_java_performance() {
    //     Stopwatch.resetAndStart();
    //     Stopwatch.measure();
    // }


    // CHECKCAST
    public static boolean test_java() {
        return rtcbenchmark_measure_java_performance();
    }
    public static boolean rtcbenchmark_measure_java_performance() {
        Stopwatch.resetAndStart();

        Object obj = new Object();
        Exception ex;
        try {
            throw new Exception();
            // ex = (Exception)obj;
        } catch (Exception ex2) {
            Stopwatch.measure();
            return true;
        }
    }

}
