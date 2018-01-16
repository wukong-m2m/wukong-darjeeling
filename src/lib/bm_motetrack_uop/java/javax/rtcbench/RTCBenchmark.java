package javax.rtcbench;

import javax.rtc.RTC;

public class RTCBenchmark {
    private final static short NUMNUMBERS = 256;

    public static String name = "MOTETRACK UOP";
    public static native void test_native();
    public static boolean test_java() {
        MobileMoteM.motetrack_init_benchmark();
        Point p = rtcbenchmark_measure_java_performance();

        RTC.avroraPrintShort(p.x);
        RTC.avroraPrintShort(p.y);
        RTC.avroraPrintShort(p.z);

        return (p.x == 210 && p.y == 249 && p.z == 0);
    }
    
    public static Point rtcbenchmark_measure_java_performance() {
        return MobileMoteM.estLocAndSend();
    }
}
