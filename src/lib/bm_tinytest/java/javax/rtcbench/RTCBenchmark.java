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
    //     RTC.startBenchmarkMeasurement_AOT();
    //     RTC.stopBenchmarkMeasurement();
    // }


    // CHECKCAST
    public static boolean test_java() {
        return rtcbenchmark_measure_java_performance();
    }
    public static boolean rtcbenchmark_measure_java_performance() {
        RTC.startBenchmarkMeasurement_AOT();

        Object obj = new Object();
        Exception ex;
        try {
            throw new Exception();
            // ex = (Exception)obj;
        } catch (Exception ex2) {
            RTC.stopBenchmarkMeasurement();
            return true;
        }
    }

}
