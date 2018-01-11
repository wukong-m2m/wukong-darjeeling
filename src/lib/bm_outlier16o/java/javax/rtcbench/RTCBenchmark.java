package javax.rtcbench;

import javax.rtc.RTC;

public class RTCBenchmark {
    private final static short NUMNUMBERS = 256;

    public static String name = "OUTLIER 16 OPT";
    public static native void test_native();
    public static boolean test_java() {
        short buffer[] = new short[NUMNUMBERS];
        boolean outliers[] = new boolean[NUMNUMBERS];

        // Fill the array
        for (int i=0; i<NUMNUMBERS; i++) {
            buffer[i] = (short)(NUMNUMBERS - 128 + i); // [-128, 127]
        }
        // Add some outliers
        buffer[2]   = 1000;
        buffer[18]  = -1000;
        buffer[200] = 1000;

        rtcbenchmark_measure_java_performance(NUMNUMBERS, buffer, (short)500, outliers);

        for (int i=0; i<NUMNUMBERS; i++) {
            switch (i) {
                case 2:
                case 18:
                case 200:
                    if (outliers[i] == false) {
                        return false;
                    }
                    break;
                default:
                    if (outliers[i] == true) {
                        return false;
                    }
                    break;
            }
        }

        return true;
    }


    public static void rtcbenchmark_measure_java_performance(short NUMNUMBERS, short[] buffer, short distance_threshold, boolean[] outliers) {
        short half_NUMNUMBERS = (short)(NUMNUMBERS >> 1);
        for (short i=0; i<NUMNUMBERS; i++) {
            short exceed_threshold_count = 0;
            for (short j=0; j<NUMNUMBERS; j++) {
                short diff = (short)(buffer[i] - buffer[j]);
                if (diff > distance_threshold || -diff > distance_threshold) {
                    exceed_threshold_count++;
                }
            }
            if (exceed_threshold_count > half_NUMNUMBERS) {
                outliers[i] = true;
            } else {
                outliers[i] = false;
            }
        }
    }
}
