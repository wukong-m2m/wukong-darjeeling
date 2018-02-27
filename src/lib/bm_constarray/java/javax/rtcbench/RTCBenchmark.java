package javax.rtcbench;

import javax.rtc.RTC;

public class RTCBenchmark {
    public static String name = "CONST ARRAY";
    public static native void test_native();

    public static boolean test_java() {
        return rtcbenchmark_measure_java_performance(2);
    }
    public static boolean rtcbenchmark_measure_java_performance(int x) {
        System.out.println("" + DataA.constant_byte_data[0]);
        System.out.println("" + DataA.constant_byte_data[1]);
        System.out.println("" + DataA.constant_byte_data[2]);
        System.out.println("" + DataA.constant_byte_data[3]);
        System.out.println("" + DataA.constant_byte_data[4]);
        System.out.println("" + DataA.constant_byte_data[5]);
        System.out.println("" + DataA.constant_byte_data[6]);
        System.out.println("" + DataB.constant_int_data[0]);
        System.out.println("" + DataB.constant_int_data[1]);
        System.out.println("" + DataB.constant_int_data[2]);
        System.out.println("" + DataB.constant_int_data[3]);
        System.out.println("" + DataB.constant_int_data[4]);
        System.out.println("" + DataB.constant_int_data[5]);
        System.out.println("" + DataA.constant_byte_data[x+1]);
    	System.out.println("" + DataB.constant_int_data[x+2]);
        return DataA.constant_byte_data[x+1] == 13
            && DataB.constant_int_data[x+2] == 8476;
    }
}
