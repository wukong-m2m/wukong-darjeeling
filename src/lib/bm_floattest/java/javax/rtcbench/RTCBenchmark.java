package javax.rtcbench;

import javax.rtc.RTC;

public class RTCBenchmark {
    public static String name = "FLOAT TEST";
    public static native void test_native();

    // Nothing here. This is just a C test to measure the cost of floating point operations.

    public static boolean test_java() {
        return rtcbenchmark_measure_java_performance();
    }
    public static boolean rtcbenchmark_measure_java_performance() {
        RTC.startBenchmarkMeasurement_AOT();
        RTC.stopBenchmarkMeasurement();
        return true;
    }
}
