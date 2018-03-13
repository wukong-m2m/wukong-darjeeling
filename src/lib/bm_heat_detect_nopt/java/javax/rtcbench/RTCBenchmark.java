package javax.rtcbench;

import javax.rtc.RTC;
import javax.rtc.Lightweight;

public class RTCBenchmark {
    public static String name = "HEAT DETECTION NOPT";
    public static native void test_native();
    public static boolean test_java() {
        HeatCalib.ACal = new short[64];
        HeatCalib.QCal = new int[64];
        HeatCalib.stdCal = new short[64];
        HeatCalib.zscore = new short[64];

        HeatDetect.zscoreWeight = new boolean[64];

        HeatCalib.heat_calib();

        short[] frame_buffer = new short[64];
        byte[] color         = new byte[64];
        byte[] rColor        = new byte[64];
        int[] largestSubset  = new int[64];
        int[] testset        = new int[64];
        int[] result         = new int[64];

        rtcbenchmark_measure_java_performance(frame_buffer, color, rColor, largestSubset, testset, result);

        if (HeatDetect.x_weight_coordinate != 466) {
            RTC.beep(0);
            RTC.avroraPrintInt(HeatDetect.x_weight_coordinate);
            return false;
        }
        if (HeatDetect.y_weight_coordinate != 222) {
            RTC.beep(1);
            RTC.avroraPrintInt(HeatDetect.y_weight_coordinate);
            return false;
        }
        if (HeatDetect.xh_weight_coordinate != -1) {
            RTC.beep(2);
            RTC.avroraPrintInt(HeatDetect.xh_weight_coordinate);
            return false;
        }
        if (HeatDetect.yh_weight_coordinate != -1) {
            RTC.beep(3);
            RTC.avroraPrintInt(HeatDetect.yh_weight_coordinate);
            return false;
        }
        if(HeatDetect.yellowGroupH != 0x00000000
                || HeatDetect.yellowGroupL != 0x0C020000
                || HeatDetect.orangeGroupH != 0x00000000
                || HeatDetect.orangeGroupL != 0x000C0000
                || HeatDetect.redGroupH != 0x00000000
                || HeatDetect.redGroupL != 0x00000000) {
            RTC.avroraPrintHex32(HeatDetect.yellowGroupH);
            RTC.avroraPrintHex32(HeatDetect.yellowGroupL);
            RTC.avroraPrintHex32(HeatDetect.orangeGroupH);
            RTC.avroraPrintHex32(HeatDetect.orangeGroupL);
            RTC.avroraPrintHex32(HeatDetect.redGroupH);
            RTC.avroraPrintHex32(HeatDetect.redGroupL);
            RTC.beep(4);
            return false;
        }

        return true;
    }

    public static void rtcbenchmark_measure_java_performance(short[] frame_buffer, byte[] color, byte[] rColor, int[] largestSubset, int[] testset, int[] result) {
        for (short i=0; i<25; i++) {
            HeatCalib.get_heat_sensor_data(frame_buffer, (short)(101+i)); // detection frames start after 100 calibration frames and 1 check frame.
            HeatDetect.heat_detect(frame_buffer, color, rColor, largestSubset, testset, result);
        }
    }
}
