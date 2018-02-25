package javax.rtcbench;

import javax.rtc.RTC;

public class RTCBenchmark {
    public static String name = "CONST ARRAY";
    public static native void test_native();

    public static boolean test_java() {
        return rtcbenchmark_measure_java_performance(2);
    }
    public static boolean rtcbenchmark_measure_java_performance(int x) {
        return DataA.constant_byte_data[x+1] == 13
            && DataB.constant_int_data[x+2] == 8476;
    }
}
