package javax.rtcbench;

import javax.rtc.RTC;

public class RTCBenchmark {
    private final static short NUMNUMBERS = 256;

    public static String name = "BUBBLESORT OPTIMISED";
    public static native void test_native();
    public static boolean test_java() {
        int numbers[] = new int[NUMNUMBERS]; // Not including this in the timing since we can't do it in C

        // Fill the array
        for (int i=0; i<NUMNUMBERS; i++)
            numbers[i] = NUMNUMBERS - 1 - i;

        // Then sort it
        rtcbenchmark_measure_java_performance(numbers);

        for (int k=0; k<NUMNUMBERS; k++) {
            if (numbers[k] != k) {
                return false;
            }
        }

        return true;
    }

    // public static void do_bubblesort_original(short[] numbers) {
    //  for (int i=0; i<NUMNUMBERS; i++) {
    //      for (int j=0; j<NUMNUMBERS-i-1; j++) {
    //          if (numbers[j]>numbers[j+1]) {
    //              short temp = numbers[j];
    //              numbers[j] = numbers[j+1];
    //              numbers[j+1] = temp;
    //          }
    //      }
    //  }
    // }


    public static void rtcbenchmark_measure_java_performance(int[] numbers) {
        for (short i=0; i<NUMNUMBERS; i++) {
            short x=(short)(NUMNUMBERS-i-1); // This doesn't get optimised the way I expected it would. Without this extra variable, it will calculate NUMNUMBERS-i-1 on each interation of the inner loop! (speedup 14.7M -> 14.2M cycles)
            short j_plus_one = 1; // Same goes for "j+1"
            for (short j=0; j<x; j++) {
                int val_at_j = numbers[j];
                int val_at_j_plus_one = numbers[j_plus_one];
                if (val_at_j>val_at_j_plus_one) {
                    numbers[j] = val_at_j_plus_one;
                    numbers[j_plus_one] = val_at_j;
                }
                j_plus_one++;
            }
        }
    }
}
