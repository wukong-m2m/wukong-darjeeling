package javax.rtcbench;

import javax.rtc.RTC;

public class RTCBenchmark {
    public static String name = "FUNCALL";
    public static native void test_native();
    public static boolean test_java() {
        rtcbenchmark_measure_java_performance();
        return true;
    }

    // 10x10 x20 nested calls -> total 2000 calls
    // Remember to compile with -Pno-proguard, or the entire benchmark will be optimised away by Proguard
    public static void rtcbenchmark_measure_java_performance() {
        // test_static();
        test_virtual(); // Need to reduce heap size in config.h to allow for slightly larger stack.
    }

    public static void test_static() {
        for (short i=0; i<10; i++) {
            static_nothing0();
            static_nothing0();
            static_nothing0();
            static_nothing0();
            static_nothing0();
            static_nothing0();
            static_nothing0();
            static_nothing0();
            static_nothing0();
            static_nothing0();
        }
    }

    public static void test_virtual() {
        RTCBenchmark obj = new RTCBenchmark();

        for (short i=0; i<1; i++) {
            obj.virtual_nothing0();
            obj.virtual_nothing0();
            obj.virtual_nothing0();
            obj.virtual_nothing0();
            obj.virtual_nothing0();
            obj.virtual_nothing0();
            obj.virtual_nothing0();
            obj.virtual_nothing0();
            obj.virtual_nothing0();
            obj.virtual_nothing0();
        }
    }
    public static void static_nothing0() { static_nothing1(); }
    public static void static_nothing1() { static_nothing2(); }
    public static void static_nothing2() { static_nothing3(); }
    public static void static_nothing3() { static_nothing4(); }
    public static void static_nothing4() { static_nothing5(); }
    public static void static_nothing5() { static_nothing6(); }
    public static void static_nothing6() { static_nothing7(); }
    public static void static_nothing7() { static_nothing8(); }
    public static void static_nothing8() { static_nothing9(); }
    public static void static_nothing9() { static_nothing10(); }
    public static void static_nothing10() { static_nothing11(); }
    public static void static_nothing11() { static_nothing12(); }
    public static void static_nothing12() { static_nothing13(); }
    public static void static_nothing13() { static_nothing14(); }
    public static void static_nothing14() { static_nothing15(); }
    public static void static_nothing15() { static_nothing16(); }
    public static void static_nothing16() { static_nothing17(); }
    public static void static_nothing17() { static_nothing18(); }
    public static void static_nothing18() { static_nothing19(); }
    public static void static_nothing19() {}


    public void virtual_nothing0() { virtual_nothing1(); }
    public void virtual_nothing1() { virtual_nothing2(); }
    public void virtual_nothing2() { virtual_nothing3(); }
    public void virtual_nothing3() { virtual_nothing4(); }
    public void virtual_nothing4() { virtual_nothing5(); }
    public void virtual_nothing5() { virtual_nothing6(); }
    public void virtual_nothing6() { virtual_nothing7(); }
    public void virtual_nothing7() { virtual_nothing8(); }
    public void virtual_nothing8() { virtual_nothing9(); }
    public void virtual_nothing9() { virtual_nothing10(); }
    public void virtual_nothing10() { virtual_nothing11(); }
    public void virtual_nothing11() { virtual_nothing12(); }
    public void virtual_nothing12() { virtual_nothing13(); }
    public void virtual_nothing13() { virtual_nothing14(); }
    public void virtual_nothing14() { virtual_nothing15(); }
    public void virtual_nothing15() { virtual_nothing16(); }
    public void virtual_nothing16() { virtual_nothing17(); }
    public void virtual_nothing17() { virtual_nothing18(); }
    public void virtual_nothing18() { virtual_nothing19(); }
    public void virtual_nothing19() {}}
