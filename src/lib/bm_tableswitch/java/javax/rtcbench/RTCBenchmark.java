package javax.rtcbench;

import javax.darjeeling.Stopwatch;

import javax.rtc.RTC;

//// UNITTEST

public class RTCBenchmark {
    public static String name = "TEST TABLESWITCH";
    public static native void test_native();

    public static boolean test_java() {
        return rtcbenchmark_measure_java_performance();
    }

    public static boolean rtcbenchmark_measure_java_performance() {
        boolean success = true;

        success = success && testTableSwitchNegative(-100000009) == 41;
        success = success && testTableSwitchNegative(-100000008) == -100000008; // 0xFA0A1EF8
        success = success && testTableSwitchNegative(-100000005) == -100000005;
        success = success && testTableSwitchNegative(-100000001) == -100000001;
        success = success && testTableSwitchNegative(-100000000) == 41;
        success = success && testTableSwitchNegative(0)          == 41;
        success = success && testTableSwitchNegative(100000001)  == 41;

        success = success && testTableSwitchZero(-100000000)     == 42;
        success = success && testTableSwitchZero(-1)             == 42;
        success = success && testTableSwitchZero(0)              == 0;
        success = success && testTableSwitchZero(5)              == 5;
        success = success && testTableSwitchZero(7)              == 7;
        success = success && testTableSwitchZero(8)              == 42;
        success = success && testTableSwitchZero(100000000)      == 42;

        success = success && testTableSwitchPositive(100000000)  == 43;
        success = success && testTableSwitchPositive(100000001)  == 100000001;
        success = success && testTableSwitchPositive(100000005)  == 100000005;
        success = success && testTableSwitchPositive(100000008)  == 100000008;
        success = success && testTableSwitchPositive(100000009)  == 43;
        success = success && testTableSwitchPositive(0)          == 43;
        success = success && testTableSwitchPositive(1100000001) == 43;

        return success;
    }

    public static int testTableSwitchNegative(int x) {
        int retval;
        switch (x) {
            case -100000008: retval = -100000008; break;
            case -100000007: retval = -100000007; break;
            case -100000006: retval = -100000006; break;
            case -100000005: retval = -100000005; break;
            case -100000004: retval = -100000004; break;
            case -100000003: retval = -100000003; break;
            case -100000002: retval = -100000002; break;
            case -100000001: retval = -100000001; break;
            default: retval = 41; break;
        }
        System.out.println("retval : " + retval);
        return retval;
    }


    public static int testTableSwitchZero(int x) {
        int retval;
        switch (x) {
            case 0: retval = 0; break;
            case 1: retval = 1; break;
            case 2: retval = 2; break;
            case 3: retval = 3; break;
            case 4: retval = 4; break;
            case 5: retval = 5; break;
            case 6: retval = 6; break;
            case 7: retval = 7; break;
            default: retval = 42; break;
        }
        System.out.println("retval : " + retval);
        return retval;
    }

    public static int testTableSwitchPositive(int x) {
        int retval;
        switch (x) {
            case 100000001: retval = 100000001; break;
            case 100000002: retval = 100000002; break;
            case 100000003: retval = 100000003; break;
            case 100000004: retval = 100000004; break;
            case 100000005: retval = 100000005; break;
            case 100000006: retval = 100000006; break;
            case 100000007: retval = 100000007; break;
            case 100000008: retval = 100000008; break;
            default: retval = 43; break;
        }
        System.out.println("retval : " + retval);
        return retval;
    }
}
