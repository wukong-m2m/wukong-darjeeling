package javax.rtcbench;

import javax.rtc.RTC;
import javax.rtc.Lightweight;

public class HeatCalib {
    public static short[] ACal;
    public static int[] QCal;
    public static short[] stdCal;
    public static short[] zscore;
    public static short z_min, z_max;

    @Lightweight
    public static native void get_heat_sensor_data(short[] frame_buffer, short frame_number);

    public static void heat_calib() {
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
    @Lightweight
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
