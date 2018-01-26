package javax.rtcbench;

import javax.rtc.RTC;
import javax.rtc.Lightweight;

public class RTCBenchmark {
    private final static short NUMNUMBERS = 256;

    private static short[] ACal;
    private static int[] QCal;
    private static short[] stdCal;
    private static short[] zscore;
    private static short z_min, z_max;

    @Lightweight
    public static native void get_heat_sensor_data(short[] frame_buffer, short frame_number);

    public static String name = "HEAT CALIBRATION";
    public static native void test_native();
    public static boolean test_java() {
        ACal = new short[64];
        QCal = new int[64];
        stdCal = new short[64];
        zscore = new short[64];

        rtcbenchmark_measure_java_performance();

        short [] expectedStdDev = { (short)26, (short)25, (short)24, (short)27, (short)24, (short)24, (short)27, (short)34, (short)27, (short)24, (short)26, (short)25, (short)23, (short)21, (short)28, (short)29, (short)25, (short)22, (short)22, (short)22, (short)25, (short)20, (short)23, (short)28, (short)24, (short)22, (short)25, (short)21, (short)22, (short)23, (short)24, (short)28, (short)23, (short)24, (short)25, (short)21, (short)25, (short)23, (short)23, (short)29, (short)22, (short)23, (short)27, (short)24, (short)24, (short)25, (short)25, (short)27, (short)30, (short)32, (short)24, (short)24, (short)25, (short)26, (short)31, (short)31, (short)34, (short)30, (short)29, (short)24, (short)26, (short)27, (short)33, (short)33 };
        short [] expectedZScore = { -76, 4, -116, 55, 12, -120, -18, 58, -48, -162, 15, -20, -47, -23, -203, -13, -140, -27, 59, 54, -56, -75, -69, 207, 58, 72, 132, 71, 177, -39, -50, 35, 247, -125, 76, -4, -80, 56, 21, -65, 195, -86, -11, -116, -100, 12, -100, -151, 36, -50, -4, -112, -100, -26, 87, 61, -91, 146, -72, 183, -130, 148, 93, 266 };
        for (short i=0; i<64; i++) {
            if (stdCal[i] != expectedStdDev[i] || zscore[i] != expectedZScore[i]) {
                return false;
            }
        }
        if (z_min != -203 || z_max != 266) {
            return false;
        }

        return true;
    }

    public static void rtcbenchmark_measure_java_performance() {
        short[] frame_buffer = new short[64];

        for (short i=0; i<100; i++) {
            get_heat_sensor_data(frame_buffer, i);
            fast_calibration(frame_buffer, i);
        }
        get_heat_sensor_data(frame_buffer, (short)100);
        zscoreCalculation(frame_buffer);
    }

    // #define rounding_int_division(dividend, divisor) (((dividend) + ((divisor) / 2)) / (divisor))
    private static void fast_calibration(short[] frame_buffer, short frame_number) {
        short frame_number_plus_one = (short)(frame_number+1);
        for(short i=0; i<64; i++) {
            short previous_ACal = ACal[i];
            ACal[i] += (frame_buffer[i] - ACal[i] + (frame_number_plus_one >>> 1)) / frame_number_plus_one;
            QCal[i] += (frame_buffer[i] - previous_ACal) * (frame_buffer[i] - ACal[i]);
        }
        for(short i=0; i<64; i++) {
            stdCal[i] = isqrt(QCal[i]/frame_number_plus_one);
        }
    }

    // http://www.cc.utah.edu/~nahaj/factoring/isqrt.c.html
    private static short isqrt (int x) {
        int   squaredbit, remainder, root;

        if (x<1) return 0;

        /* Load the binary constant 01 00 00 ... 00, where the number
        * of zero bits to the right of the single one bit
        * is even, and the one bit is as far left as is consistant
        * with that condition.)
        */
        squaredbit  = (int) ((((int) ~0L) >>> 1) & 
                        ~(((int) ~0L) >>> 2));
        /* This portable load replaces the loop that used to be 
        * here, and was donated by  legalize@xmission.com 
        */

        /* Form bits of the answer. */
        remainder = x;  root = 0;
        while (squaredbit > 0) {
            if (remainder >= (squaredbit | root)) {
                remainder -= (squaredbit | root);
                root >>= 1; root |= squaredbit;
            } else {
                root >>= 1;
            }
            squaredbit >>= 2; 
        }

        return (short)root;
    }

    private static void zscoreCalculation(short[] frame_buffer) {
        short tempMax = -30000;
        short tempMin = 30000;

        for(int i=0; i<64; i++) {
            short score = (short)(100 * (frame_buffer[i] - ACal[i]) / stdCal[i]);

            zscore[i] = score;

            if(score > tempMax) {
                tempMax = score;
            }

            if(score < tempMin) {
                tempMin = score;
            }
        }

        z_max = tempMax;
        z_min = tempMin;
    }
}
