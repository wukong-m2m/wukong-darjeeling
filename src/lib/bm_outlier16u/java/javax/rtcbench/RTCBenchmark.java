package javax.rtcbench;

import javax.rtc.RTC;

public class RTCBenchmark {
    private final static short NUMNUMBERS = 20;

    public static String name = "OUTLIER 16 UNOPT";
    public static native void test_native();
    public static boolean test_java() {
        short buffer[] = new short[NUMNUMBERS];
        short distance_matrix[] = new short[NUMNUMBERS*NUMNUMBERS];
        boolean outliers[] = new boolean[NUMNUMBERS];

        // Fill the array
        for (int i=0; i<NUMNUMBERS; i++) {
            buffer[i] = (short)(NUMNUMBERS - 128 + i); // [-128, 127]
        }
        // Add some outliers
        buffer[2]   = 1000;
        buffer[11]  = -1000;

        rtcbenchmark_measure_java_performance(NUMNUMBERS, buffer, distance_matrix, (short)500, outliers);

        for (int i=0; i<NUMNUMBERS; i++) {
            switch (i) {
                case 2:
                case 11:
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

        RTC.beep(1);
        return true;
    }


    public static void rtcbenchmark_measure_java_performance(short NUMNUMBERS, short[] buffer, short[] distance_matrix, short distance_threshold, boolean[] outliers) {
        short k=0; // Since we scan one line at a time, we don't need to calculate a matrix index.
                   // The first NUMNUMBERS distances correspond to measurement 1, the second NUMNUMBERS distances to measurement 2, etc.
        for (short i=0; i<NUMNUMBERS; i++) {
            for (short j=0; j<NUMNUMBERS; j++) {
                distance_matrix[k++] = (short)(buffer[i] - buffer[j]);
            }
        }
        
        k=0;
        for (short i=0; i<NUMNUMBERS; i++) {
            short exceed_threshold_count = 0;
            for (short j=0; j<NUMNUMBERS; j++) {
                short diff = distance_matrix[k++];
                if (diff > distance_threshold || (short)(-diff) > distance_threshold) {
                    exceed_threshold_count++;
                }
            }
            if (exceed_threshold_count > (NUMNUMBERS >> 1)) {
                outliers[i] = true;
            } else {
                outliers[i] = false;
            }
        }
    }
}
