package javax.rtcbench;

import javax.darjeeling.Stopwatch;

public class RTCBenchmark {
    public static String name = "FUNCALL";
    public static native void test_native();
    public static boolean test_java() {
        rtcbenchmark_measure_java_performance();
        return true;
    }

    // 1000 single calls
    // public static void rtcbenchmark_measure_java_performance() {
    //     Stopwatch.resetAndStart();

    //     for (short i=0; i<100; i++) {
    //         nothing15();
    //         nothing15();
    //         nothing15();
    //         nothing15();
    //         nothing15();
    //         nothing15();
    //         nothing15();
    //         nothing15();
    //         nothing15();
    //         nothing15();
    //     }

    //     Stopwatch.measure();
    // }


    // 100x16 nested calls
    // Current cycles: 1048436, max stack: 810
    public static void rtcbenchmark_measure_java_performance() {
        Stopwatch.resetAndStart();

        for (short i=0; i<10; i++) {
            nothing0();
            nothing0();
            nothing0();
            nothing0();
            nothing0();
            nothing0();
            nothing0();
            nothing0();
            nothing0();
            nothing0();
        }

        Stopwatch.measure();
    }

    public static void nothing0() { nothing1(); }
    public static void nothing1() { nothing2(); }
    public static void nothing2() { nothing3(); }
    public static void nothing3() { nothing4(); }
    public static void nothing4() { nothing5(); }
    public static void nothing5() { nothing6(); }
    public static void nothing6() { nothing7(); }
    public static void nothing7() { nothing8(); }
    public static void nothing8() { nothing9(); }
    public static void nothing9() { nothing10(); }
    public static void nothing10() { nothing11(); }
    public static void nothing11() { nothing12(); }
    public static void nothing12() { nothing13(); }
    public static void nothing13() { nothing14(); }
    public static void nothing14() { nothing15(); }
    public static void nothing15() { }
}
