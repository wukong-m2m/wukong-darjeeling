package javax.rtcbench;

import javax.rtc.RTC;

public class RTCBenchmark {
    public static String name = "CONST ARRAY";
    public static native void test_native();

    public static boolean test_java() {
        return rtcbenchmark_measure_java_performance();
    }
    public static boolean rtcbenchmark_measure_java_performance() {
        return Data.constant_data[3] == 13;
    }
}
