package javax.rtcbench;

import javax.rtc.RTC;

public class RTCBenchmark {
    private final static short NUMNUMBERS = 20;

    public static String name = "OUTLIER 16 UNOPT NOPT";
    public static native void test_native();
    public static boolean test_java() {
        short buffer[] = new short[NUMNUMBERS];
        short distance_matrix[] = new short[NUMNUMBERS*NUMNUMBERS];
        boolean outliers[] = new boolean[NUMNUMBERS];

        // Fill the array
        for (short i=0; i<NUMNUMBERS; i++) {
            buffer[i] = i;
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
        // Calculate distance matrix
        short sub_start=0;
        for (short i=0; i<NUMNUMBERS; i++) {
            short hor = sub_start;
            short ver = sub_start;
            for (short j=i; j<NUMNUMBERS; j++) {
                if (buffer[i] > buffer[j]) {
                    short diff = (short)(buffer[i] - buffer[j]);
                    distance_matrix[hor] = diff;
                    distance_matrix[ver] = diff;
                } else {
                    short diff = (short)(buffer[j] - buffer[i]);
                    distance_matrix[hor] = diff;
                    distance_matrix[ver] = diff;
                }

                hor ++;
                ver += NUMNUMBERS;
            }
            sub_start+=NUMNUMBERS+1;
        }

        // Determine outliers
        short k=0; // Since we scan one line at a time, we don't need to calculate a matrix index.
                   // The first NUMNUMBERS distances correspond to measurement 1, the second NUMNUMBERS distances to measurement 2, etc.
        if (distance_threshold > 0) {
            for (short i=0; i<NUMNUMBERS; i++) {
                short exceed_threshold_count = 0;
                for (short j=0; j<NUMNUMBERS; j++) {
                    if (distance_matrix[k] < 0 || distance_matrix[k] > distance_threshold) {
                        exceed_threshold_count++;
                    }
                    k++;
                }

                if (exceed_threshold_count > (NUMNUMBERS >> 1)) {
                    outliers[i] = true;
                } else {
                    outliers[i] = false;
                }
            }
        } else {
            for (short i=0; i<NUMNUMBERS; i++) {
                short exceed_threshold_count = 0;
                for (short j=0; j<NUMNUMBERS; j++) {
                    if (distance_matrix[k] < 0 && distance_matrix[k] > distance_threshold) {
                        exceed_threshold_count++;
                    }
                    k++;
                }

                if (exceed_threshold_count > (NUMNUMBERS >> 1)) {
                    outliers[i] = true;
                } else {
                    outliers[i] = false;
                }
            }
        }
    }
}
