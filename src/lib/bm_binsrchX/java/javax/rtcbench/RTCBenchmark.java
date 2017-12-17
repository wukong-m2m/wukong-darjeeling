package javax.rtcbench;

import javax.rtc.RTC;

// Based on binary search benchmark in TakaTuka.
// http://sourceforge.net/p/takatuka/code/HEAD/tree/
// Should be the same code that Joshua Ellul used for his thesis.
// Modified slightly to do the same test Joshua did:
// "The binary search test performs 1,000 binary searches for the worst case search in 100 16 bit values." on p75 of his thesis

public class RTCBenchmark {
    private final static short NUMNUMBERS = 100;

    public static String name = "BINARY SEARCH NOT OPTIMISED";
    public static native void test_native();
    public static boolean test_java() {
        short numbers[] = new short[NUMNUMBERS]; // Not including this in the timing since we can't do it in C

        // Fill the array
        for (int loop = 0; loop < NUMNUMBERS; loop++) {
            numbers[loop] = (short) (loop - 30);
        }

        rtcbenchmark_measure_java_performance(numbers);

        return true;
    }

    public static void rtcbenchmark_measure_java_performance(short[] numbers) {
        short toFind = (short)(numbers[0] - 1);

        for (short i=0; i<1000; i++) {
            short low = 0;
            short high = (short)(numbers.length - 1);
            short mid;
            while (low <= high) {
                mid = (short)((low + high) / 2);
                if (numbers[mid] < toFind) {
                    low = (short)(mid + 1);
                } else if (numbers[mid] > toFind) {
                    high = (short)(mid - 1);
                } else {
                    break; // Found. Would return from here in a normal search, but for this benchmark we just want to try many numbers.
                }
            }
        }
    }
}
