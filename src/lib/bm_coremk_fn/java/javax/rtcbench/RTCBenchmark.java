package javax.rtcbench;
import javax.rtc.RTC;

import javax.darjeeling.Stopwatch;

public class RTCBenchmark {
    public static String name = "COREMARK";
    public static native void test_native();
    public static boolean test_java() {
        return CoreMain.core_mark_main();
    }

    public static void nothing() {}
}
