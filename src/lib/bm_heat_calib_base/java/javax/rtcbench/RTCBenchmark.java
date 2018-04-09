package javax.rtcbench;

import javax.rtc.RTC;
import javax.rtc.Lightweight;

public class RTCBenchmark {
    public static String name = "HEAT CALIBRATION BASE";
    public static native void test_native();
    public static boolean test_java() {
        HeatCalib.ACal = new short[64];
        HeatCalib.QCal = new int[64];
        HeatCalib.stdCal = new short[64];
        HeatCalib.zscore = new short[64];

        rtcbenchmark_measure_java_performance();

        short [] expectedStdDev = { (short)26, (short)25, (short)24, (short)27, (short)24, (short)24, (short)27, (short)34, (short)27, (short)24, (short)26, (short)25, (short)23, (short)21, (short)28, (short)29, (short)25, (short)22, (short)22, (short)22, (short)25, (short)20, (short)23, (short)28, (short)24, (short)22, (short)25, (short)21, (short)22, (short)23, (short)24, (short)28, (short)23, (short)24, (short)25, (short)21, (short)25, (short)23, (short)23, (short)29, (short)22, (short)23, (short)27, (short)24, (short)24, (short)25, (short)25, (short)27, (short)30, (short)32, (short)24, (short)24, (short)25, (short)26, (short)31, (short)31, (short)34, (short)30, (short)29, (short)24, (short)26, (short)27, (short)33, (short)33 };
        short [] expectedZScore = { -76, 4, -116, 55, 12, -120, -18, 58, -48, -162, 15, -20, -47, -23, -203, -13, -140, -27, 59, 54, -56, -75, -69, 207, 58, 72, 132, 71, 177, -39, -50, 35, 247, -125, 76, -4, -80, 56, 21, -65, 195, -86, -11, -116, -100, 12, -100, -151, 36, -50, -4, -112, -100, -26, 87, 61, -91, 146, -72, 183, -130, 148, 93, 266 };
        for (short i=0; i<64; i++) {
            if (HeatCalib.stdCal[i] != expectedStdDev[i] || HeatCalib.zscore[i] != expectedZScore[i]) {
                return false;
            }
        }
        if (HeatCalib.z_min != -203 || HeatCalib.z_max != 266) {
            return false;
        }

        return true;
    }

    public static void rtcbenchmark_measure_java_performance() {
        HeatCalib.heat_calib();
    }
}
