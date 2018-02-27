package javax.rtcbench;

import javax.rtc.*;

public class RTCBenchmark {
    public static String name = "FIX_FFT 16 FN";
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

    public static short fix_mpy(short a, short b) {
        return (short)(((int)(a) * (int)(b))>>15);
    }

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
                wr =  Sinewave.data[j+N_WAVE/4];
                wi = (short)(-Sinewave.data[j]);

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
                    // tr =  (short)(   ((short) ((wr*frj)>>15)) - ((short) ((wi*fij)>>15))      );
                    // ti =  (short)(   ((short) ((wr*fij)>>15)) + ((short) ((wi*frj)>>15))      );
                    tr = (short)(fix_mpy(wr,frj) - fix_mpy(wi,fij));
                    ti = (short)(fix_mpy(wr,fij) + fix_mpy(wi,frj));

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
