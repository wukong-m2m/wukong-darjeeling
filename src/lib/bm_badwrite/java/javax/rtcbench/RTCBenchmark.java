package javax.rtcbench;

import javax.rtc.RTC;

public class RTCBenchmark {
    public static String name = "BAD WRITE";
    public static native void test_native();

    public static boolean test_java() {

        return rtcbenchmark_measure_java_performance();

    }

    public static boolean rtcbenchmark_measure_java_performance() {
        byte numbers[] = new byte[100];
        for (short i=0; i<30000; i++) {
            RTC.avroraPrintInt(i);
            numbers[-i] = (byte)0;
        }

        return true;
    }
}
