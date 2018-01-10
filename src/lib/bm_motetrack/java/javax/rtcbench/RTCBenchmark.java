package javax.rtcbench;

import javax.rtc.RTC;

public class RTCBenchmark {
    private final static short NUMNUMBERS = 256;

    public static String name = "MOTETRACK";
    public static native void test_native();
    public static boolean test_java() {
    	NewSignature.init();

        return true;
    }
    
    public static void rtcbenchmark_measure_java_performance(int[] numbers) {

    }
}
