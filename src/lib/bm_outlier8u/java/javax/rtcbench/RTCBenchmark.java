package javax.rtcbench;

import javax.rtc.RTC;

public class RTCBenchmark {
    private final static short NUMNUMBERS = 20;

    public static String name = "OUTLIER 8 UNOPT";
    public static native void test_native();
    public static boolean test_java() {
        byte buffer[] = new byte[NUMNUMBERS];
        byte distance_matrix[] = new byte[NUMNUMBERS*NUMNUMBERS];
        boolean outliers[] = new boolean[NUMNUMBERS];

        // Fill the array
        for (int i=0; i<NUMNUMBERS; i++) {
            buffer[i] = (byte)i;
        }
        // Add some outliers
        buffer[2]   = 127;
        buffer[11]  = -128;

        rtcbenchmark_measure_java_performance(NUMNUMBERS, buffer, distance_matrix, 50, outliers);

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


    public static void rtcbenchmark_measure_java_performance(short NUMNUMBERS, byte[] buffer, byte[] distance_matrix, int distance_threshold, boolean[] outliers) {
        short k=0; // Since we scan one line at a time, we don't need to calculate a matrix index.
                   // The first NUMNUMBERS distances correspond to measurement 1, the second NUMNUMBERS distances to measurement 2, etc.
        for (short i=0; i<NUMNUMBERS; i++) {
            for (short j=0; j<NUMNUMBERS; j++) {
                distance_matrix[k++] = (byte)(buffer[i] - buffer[j]);
            }
        }
        
        k=0;
        short half_NUMNUMBERS = (short)(NUMNUMBERS >> 1);
        for (short i=0; i<NUMNUMBERS; i++) {
            short exceed_threshold_count = 0;
            for (short j=0; j<NUMNUMBERS; j++) {
                byte diff = distance_matrix[k++];
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
