package javax.rtcbench;

import javax.rtc.*;

public class RTCBenchmark {
    public static String name = "FIX_FFT 16 NCA";
    public static native void test_native();



    public static boolean test_java() {
        final int RTCTEST_FFT_ARRAYSIZE = 6;
        final int NUMNUMBERS = 1<<RTCTEST_FFT_ARRAYSIZE;
        short data[] = new short[NUMNUMBERS];
        short im[] = new short[NUMNUMBERS];

        // Fill the array
        for (int i=0; i<NUMNUMBERS; i++) {
            data[i] = (short)(i*16);
            im[i] = (short)0;
        }

        // System.out.println("BEFORE FFT"); for (int i=0; i<NUMNUMBERS; i++) { System.out.println("-----" + data[i] + " " + im[i]); }

        // Do the actual FFT
        rtcbenchmark_measure_java_performance(data, im, (short)RTCTEST_FFT_ARRAYSIZE, false);

        // System.out.println("AFTER FFT"); for (int i=0; i<NUMNUMBERS; i++) { System.out.println("-----" + data[i] + " " + im[i]); }
        
        for (int i=0; i<NUMNUMBERS; i++) {
            if (desiredOutputData(i) != data[i] || desiredOutputIm(i) != im[i]) {
                RTC.beep(0);
                RTC.avroraPrintInt(i);
                RTC.avroraPrintShort(desiredOutputData(i));
                RTC.avroraPrintShort(data[i]);
                RTC.avroraPrintShort(desiredOutputIm(i));
                RTC.avroraPrintShort(im[i]);
                return false;
            }
        }

        return true;
    }

    private static short desiredOutputData(int index) {
        switch (index) {
            case 0: return 498;
            case 1: return -10;
            case 2: return -8;
            case 3: return -10;
            case 4: return -10;
            case 5: return -9;
            case 6: return -10;
            case 7: return -10;
            case 8: return -8;
            case 9: return -8;
            case 10: return -8;
            case 11: return -9;
            case 12: return -8;
            case 13: return -9;
            case 14: return -8;
            case 15: return -9;
            case 16: return -8;
            case 17: return -8;
            case 18: return -8;
            case 19: return -8;
            case 20: return -8;
            case 21: return -8;
            case 22: return -8;
            case 23: return -9;
            case 24: return -8;
            case 25: return -8;
            case 26: return -8;
            case 27: return -7;
            case 28: return -9;
            case 29: return -8;
            case 30: return -8;
            case 31: return -8;
            case 32: return -8;
            case 33: return -8;
            case 34: return -8;
            case 35: return -8;
            case 36: return -8;
            case 37: return -7;
            case 38: return -8;
            case 39: return -8;
            case 40: return -8;
            case 41: return -8;
            case 42: return -8;
            case 43: return -7;
            case 44: return -8;
            case 45: return -7;
            case 46: return -8;
            case 47: return -7;
            case 48: return -8;
            case 49: return -8;
            case 50: return -8;
            case 51: return -8;
            case 52: return -8;
            case 53: return -8;
            case 54: return -8;
            case 55: return -7;
            case 56: return -8;
            case 57: return -10;
            case 58: return -10;
            case 59: return -9;
            case 60: return -9;
            case 61: return -10;
            case 62: return -8;
            case 63: return -10;
            default: return 0;
        }
    }
    private static short desiredOutputIm(int index) {
        switch(index) {
            case 0: return 0;
            case 1: return 158;
            case 2: return 78;
            case 3: return 49;
            case 4: return 37;
            case 5: return 29;
            case 6: return 24;
            case 7: return 19;
            case 8: return 18;
            case 9: return 14;
            case 10: return 13;
            case 11: return 11;
            case 12: return 8;
            case 13: return 9;
            case 14: return 8;
            case 15: return 7;
            case 16: return 7;
            case 17: return 6;
            case 18: return 5;
            case 19: return 4;
            case 20: return 4;
            case 21: return 4;
            case 22: return 3;
            case 23: return 2;
            case 24: return 2;
            case 25: return 2;
            case 26: return 2;
            case 27: return 1;
            case 28: return 1;
            case 29: return 0;
            case 30: return -1;
            case 31: return -1;
            case 32: return 0;
            case 33: return 0;
            case 34: return 0;
            case 35: return -1;
            case 36: return -1;
            case 37: return -1;
            case 38: return -2;
            case 39: return -3;
            case 40: return -2;
            case 41: return -2;
            case 42: return -3;
            case 43: return -5;
            case 44: return -4;
            case 45: return -5;
            case 46: return -6;
            case 47: return -7;
            case 48: return -7;
            case 49: return -8;
            case 50: return -9;
            case 51: return -10;
            case 52: return -10;
            case 53: return -12;
            case 54: return -13;
            case 55: return -16;
            case 56: return -18;
            case 57: return -20;
            case 58: return -24;
            case 59: return -29;
            case 60: return -39;
            case 61: return -50;
            case 62: return -79;
            case 63: return -159;
            default: return 0;
        }
    }

    private final static short N_WAVE = 1024;    // full length of Sinewave[]
    private final static short LOG2_N_WAVE = 10; // log2(N_WAVE)

    // Pseudo-cosine function for 2pi equalling N_WAVE = 256.
    // Shifting by "pi/2", e.g. N_WAVE/4, gives sine.

    // signed 8-bit values written into Arduino's 32k EEPROM memory
    // Technically we could get along with a quarter of this data table but I think we shouldn't.
    private final static short Sinewave[] = new short[] {
          0,    201,    402,    603,    804,   1005,   1206,   1406,
       1607,   1808,   2009,   2209,   2410,   2610,   2811,   3011,
       3211,   3411,   3611,   3811,   4011,   4210,   4409,   4608,
       4807,   5006,   5205,   5403,   5601,   5799,   5997,   6195,
       6392,   6589,   6786,   6982,   7179,   7375,   7571,   7766,
       7961,   8156,   8351,   8545,   8739,   8932,   9126,   9319,
       9511,   9703,   9895,  10087,  10278,  10469,  10659,  10849,
      11038,  11227,  11416,  11604,  11792,  11980,  12166,  12353,
      12539,  12724,  12909,  13094,  13278,  13462,  13645,  13827,
      14009,  14191,  14372,  14552,  14732,  14911,  15090,  15268,
      15446,  15623,  15799,  15975,  16150,  16325,  16499,  16672,
      16845,  17017,  17189,  17360,  17530,  17699,  17868,  18036,
      18204,  18371,  18537,  18702,  18867,  19031,  19194,  19357,
      19519,  19680,  19840,  20000,  20159,  20317,  20474,  20631,
      20787,  20942,  21096,  21249,  21402,  21554,  21705,  21855,
      22004,  22153,  22301,  22448,  22594,  22739,  22883,  23027,
      23169,  23311,  23452,  23592,  23731,  23869,  24006,  24143,
      24278,  24413,  24546,  24679,  24811,  24942,  25072,  25201,
      25329,  25456,  25582,  25707,  25831,  25954,  26077,  26198,
      26318,  26437,  26556,  26673,  26789,  26905,  27019,  27132,
      27244,  27355,  27466,  27575,  27683,  27790,  27896,  28001,
      28105,  28208,  28309,  28410,  28510,  28608,  28706,  28802,
      28897,  28992,  29085,  29177,  29268,  29358,  29446,  29534,
      29621,  29706,  29790,  29873,  29955,  30036,  30116,  30195,
      30272,  30349,  30424,  30498,  30571,  30643,  30713,  30783,
      30851,  30918,  30984,  31049,  31113,  31175,  31236,  31297,
      31356,  31413,  31470,  31525,  31580,  31633,  31684,  31735,
      31785,  31833,  31880,  31926,  31970,  32014,  32056,  32097,
      32137,  32176,  32213,  32249,  32284,  32318,  32350,  32382,
      32412,  32441,  32468,  32495,  32520,  32544,  32567,  32588,
      32609,  32628,  32646,  32662,  32678,  32692,  32705,  32717,
      32727,  32736,  32744,  32751,  32757,  32761,  32764,  32766,
      32767,  32766,  32764,  32761,  32757,  32751,  32744,  32736,
      32727,  32717,  32705,  32692,  32678,  32662,  32646,  32628,
      32609,  32588,  32567,  32544,  32520,  32495,  32468,  32441,
      32412,  32382,  32350,  32318,  32284,  32249,  32213,  32176,
      32137,  32097,  32056,  32014,  31970,  31926,  31880,  31833,
      31785,  31735,  31684,  31633,  31580,  31525,  31470,  31413,
      31356,  31297,  31236,  31175,  31113,  31049,  30984,  30918,
      30851,  30783,  30713,  30643,  30571,  30498,  30424,  30349,
      30272,  30195,  30116,  30036,  29955,  29873,  29790,  29706,
      29621,  29534,  29446,  29358,  29268,  29177,  29085,  28992,
      28897,  28802,  28706,  28608,  28510,  28410,  28309,  28208,
      28105,  28001,  27896,  27790,  27683,  27575,  27466,  27355,
      27244,  27132,  27019,  26905,  26789,  26673,  26556,  26437,
      26318,  26198,  26077,  25954,  25831,  25707,  25582,  25456,
      25329,  25201,  25072,  24942,  24811,  24679,  24546,  24413,
      24278,  24143,  24006,  23869,  23731,  23592,  23452,  23311,
      23169,  23027,  22883,  22739,  22594,  22448,  22301,  22153,
      22004,  21855,  21705,  21554,  21402,  21249,  21096,  20942,
      20787,  20631,  20474,  20317,  20159,  20000,  19840,  19680,
      19519,  19357,  19194,  19031,  18867,  18702,  18537,  18371,
      18204,  18036,  17868,  17699,  17530,  17360,  17189,  17017,
      16845,  16672,  16499,  16325,  16150,  15975,  15799,  15623,
      15446,  15268,  15090,  14911,  14732,  14552,  14372,  14191,
      14009,  13827,  13645,  13462,  13278,  13094,  12909,  12724,
      12539,  12353,  12166,  11980,  11792,  11604,  11416,  11227,
      11038,  10849,  10659,  10469,  10278,  10087,   9895,   9703,
       9511,   9319,   9126,   8932,   8739,   8545,   8351,   8156,
       7961,   7766,   7571,   7375,   7179,   6982,   6786,   6589,
       6392,   6195,   5997,   5799,   5601,   5403,   5205,   5006,
       4807,   4608,   4409,   4210,   4011,   3811,   3611,   3411,
       3211,   3011,   2811,   2610,   2410,   2209,   2009,   1808,
       1607,   1406,   1206,   1005,    804,    603,    402,    201,
          0,   -201,   -402,   -603,   -804,  -1005,  -1206,  -1406,
      -1607,  -1808,  -2009,  -2209,  -2410,  -2610,  -2811,  -3011,
      -3211,  -3411,  -3611,  -3811,  -4011,  -4210,  -4409,  -4608,
      -4807,  -5006,  -5205,  -5403,  -5601,  -5799,  -5997,  -6195,
      -6392,  -6589,  -6786,  -6982,  -7179,  -7375,  -7571,  -7766,
      -7961,  -8156,  -8351,  -8545,  -8739,  -8932,  -9126,  -9319,
      -9511,  -9703,  -9895, -10087, -10278, -10469, -10659, -10849,
     -11038, -11227, -11416, -11604, -11792, -11980, -12166, -12353,
     -12539, -12724, -12909, -13094, -13278, -13462, -13645, -13827,
     -14009, -14191, -14372, -14552, -14732, -14911, -15090, -15268,
     -15446, -15623, -15799, -15975, -16150, -16325, -16499, -16672,
     -16845, -17017, -17189, -17360, -17530, -17699, -17868, -18036,
     -18204, -18371, -18537, -18702, -18867, -19031, -19194, -19357,
     -19519, -19680, -19840, -20000, -20159, -20317, -20474, -20631,
     -20787, -20942, -21096, -21249, -21402, -21554, -21705, -21855,
     -22004, -22153, -22301, -22448, -22594, -22739, -22883, -23027,
     -23169, -23311, -23452, -23592, -23731, -23869, -24006, -24143,
     -24278, -24413, -24546, -24679, -24811, -24942, -25072, -25201,
     -25329, -25456, -25582, -25707, -25831, -25954, -26077, -26198,
     -26318, -26437, -26556, -26673, -26789, -26905, -27019, -27132,
     -27244, -27355, -27466, -27575, -27683, -27790, -27896, -28001,
     -28105, -28208, -28309, -28410, -28510, -28608, -28706, -28802,
     -28897, -28992, -29085, -29177, -29268, -29358, -29446, -29534,
     -29621, -29706, -29790, -29873, -29955, -30036, -30116, -30195,
     -30272, -30349, -30424, -30498, -30571, -30643, -30713, -30783,
     -30851, -30918, -30984, -31049, -31113, -31175, -31236, -31297,
     -31356, -31413, -31470, -31525, -31580, -31633, -31684, -31735,
     -31785, -31833, -31880, -31926, -31970, -32014, -32056, -32097,
     -32137, -32176, -32213, -32249, -32284, -32318, -32350, -32382,
     -32412, -32441, -32468, -32495, -32520, -32544, -32567, -32588,
     -32609, -32628, -32646, -32662, -32678, -32692, -32705, -32717,
     -32727, -32736, -32744, -32751, -32757, -32761, -32764, -32766,
     -32767, -32766, -32764, -32761, -32757, -32751, -32744, -32736,
     -32727, -32717, -32705, -32692, -32678, -32662, -32646, -32628,
     -32609, -32588, -32567, -32544, -32520, -32495, -32468, -32441,
     -32412, -32382, -32350, -32318, -32284, -32249, -32213, -32176,
     -32137, -32097, -32056, -32014, -31970, -31926, -31880, -31833,
     -31785, -31735, -31684, -31633, -31580, -31525, -31470, -31413,
     -31356, -31297, -31236, -31175, -31113, -31049, -30984, -30918,
     -30851, -30783, -30713, -30643, -30571, -30498, -30424, -30349,
     -30272, -30195, -30116, -30036, -29955, -29873, -29790, -29706,
     -29621, -29534, -29446, -29358, -29268, -29177, -29085, -28992,
     -28897, -28802, -28706, -28608, -28510, -28410, -28309, -28208,
     -28105, -28001, -27896, -27790, -27683, -27575, -27466, -27355,
     -27244, -27132, -27019, -26905, -26789, -26673, -26556, -26437,
     -26318, -26198, -26077, -25954, -25831, -25707, -25582, -25456,
     -25329, -25201, -25072, -24942, -24811, -24679, -24546, -24413,
     -24278, -24143, -24006, -23869, -23731, -23592, -23452, -23311,
     -23169, -23027, -22883, -22739, -22594, -22448, -22301, -22153,
     -22004, -21855, -21705, -21554, -21402, -21249, -21096, -20942,
     -20787, -20631, -20474, -20317, -20159, -20000, -19840, -19680,
     -19519, -19357, -19194, -19031, -18867, -18702, -18537, -18371,
     -18204, -18036, -17868, -17699, -17530, -17360, -17189, -17017,
     -16845, -16672, -16499, -16325, -16150, -15975, -15799, -15623,
     -15446, -15268, -15090, -14911, -14732, -14552, -14372, -14191,
     -14009, -13827, -13645, -13462, -13278, -13094, -12909, -12724,
     -12539, -12353, -12166, -11980, -11792, -11604, -11416, -11227,
     -11038, -10849, -10659, -10469, -10278, -10087,  -9895,  -9703,
      -9511,  -9319,  -9126,  -8932,  -8739,  -8545,  -8351,  -8156,
      -7961,  -7766,  -7571,  -7375,  -7179,  -6982,  -6786,  -6589,
      -6392,  -6195,  -5997,  -5799,  -5601,  -5403,  -5205,  -5006,
      -4807,  -4608,  -4409,  -4210,  -4011,  -3811,  -3611,  -3411,
      -3211,  -3011,  -2811,  -2610,  -2410,  -2209,  -2009,  -1808,
      -1607,  -1406,  -1206,  -1005,   -804,   -603,   -402,   -201,
    };

    public static short rtcbenchmark_measure_java_performance(short[] fr, short[] fi, short m, boolean inverse) {
        short mr,nn,i,j,l,k,istep, n, scale;
        boolean shift;
        short qr,qi,tr,ti,wr,wi; //,t;

        n = (short)(1<<m);

        if(n > N_WAVE)
            return -1;

        mr = 0;
        nn = (short)(n - 1);
        scale = 0;

        /* decimation in time - re-order data */
        for(m=1; m<=nn; ++m) {
            l = n;
            do {
                l >>= 1;
            } while(mr+l > nn);
            mr = (short)((mr & (l-1)) + l);

            if(mr <= m) continue;
            tr = fr[m];
            fr[m] = fr[mr];
            fr[mr] = tr;
            ti = fi[m];
            fi[m] = fi[mr];
            fi[mr] = ti;
        }

        l = 1;
        k = LOG2_N_WAVE-1;
        while(l < n) {
            if(inverse) {
                /* variable scaling, depending upon data */
                shift = false;
                for(i=0; i<n; ++i) {
                    j = fr[i];
                    if(j < 0)
                        j = (short)-j;
                    m = fi[i];
                    if(m < 0)
                        m = (short)-m;
                    if(j > 16383 || m > 16383) {
                        shift = true;
                        break;
                    }
                }
                if(shift)
                    ++scale;
            } else {
                /* fixed scaling, for proper normalization -
                there will be log2(n) passes, so this
                results in an overall factor of 1/n,
                distributed to maximize arithmetic accuracy. */
                shift = true;
            }
            /* it may not be obvious, but the shift will be performed
            on each data point exactly once, during this pass. */
            istep = (short)(l << 1);
            for(m=0; m<l; ++m) {
                j = (short)(m << k);
                /* 0 <= j < N_WAVE/2 */
                wr =  Sinewave[j+N_WAVE/4];
                wi = (short)(-Sinewave[j]);

                if(inverse)
                    wi = (short)(-wi);
                if(shift) {
                    wr >>= 1;
                    wi >>= 1;
                }

                for(i=m; i<n; i+=istep) {
                    j = (short)(i + l);

                    // Inlined fix_mpy
                    // tr = (short)(fix_mpy(wr,fr[j]) -
                    //     fix_mpy(wi,fi[j]));
                    // ti = (short)(fix_mpy(wr,fi[j]) +
                    //     fix_mpy(wi,fr[j]));

                    short frj = fr[j];
                    short fij = fi[j];
                    tr =  (short)(   ((short) ((wr*frj)>>15)) - ((short) ((wi*fij)>>15))      );
                    ti =  (short)(   ((short) ((wr*fij)>>15)) + ((short) ((wi*frj)>>15))      );

                    qr = fr[i];
                    qi = fi[i];
                    if(shift) {
                        qr >>= 1;
                        qi >>= 1;
                    }
                    fr[j] = (short)(qr - tr);
                    fi[j] = (short)(qi - ti);
                    fr[i] = (short)(qr + tr);
                    fi[i] = (short)(qi + ti);
                }
            }
            --k;
            l = istep;
        }

        return scale;
    }
}
