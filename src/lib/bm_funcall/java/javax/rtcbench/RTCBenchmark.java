package javax.rtcbench;

import javax.darjeeling.Stopwatch;

public class RTCBenchmark {
    public static String name = "FUNCALL";
    public static native void test_native();
    public static boolean test_java() {
        rtcbenchmark_measure_java_performance();
        return true;
    }

    public static void rtcbenchmark_measure_java_performance() {
        Stopwatch.resetAndStart();

        for (short i=0; i<100; i++) {
            nothing();
            nothing();
            nothing();
            nothing();
            nothing();
            nothing();
            nothing();
            nothing();
            nothing();
            nothing();
        }

        Stopwatch.measure();
    }

    public static void nothing() {}
}
