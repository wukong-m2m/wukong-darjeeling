package javax.rtcbench;

import javax.darjeeling.Stopwatch;

// Based on binary search benchmark in TakaTuka.
// http://sourceforge.net/p/takatuka/code/HEAD/tree/
// Should be the same code that Joshua Ellul used for his thesis.
// Modified slightly to do the same test Joshua did:
// "The binary search test performs 1,000 binary searches for the worst case search in 100 16 bit values." on p75 of his thesis

public class RTCBenchmark {
    private final static short NUMNUMBERS = 100;

    public static String name = "BINARY SEARCH OPTIMISED";
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
        Stopwatch.resetAndStart();

        short toFind = (short)(numbers[0] - 1);

        for (short i=0; i<1000; i++) {
            short low = 0;
            short high = (short)(numbers.length - 1);
            short mid;
            while (low <= high) {
                // mid = (low + high) / 2;
                mid = (short)((short)(low + high) >>> 1); // This is usually much faster than / 2. Any optimising compiler should know this.

                short number_mid;
                if ((number_mid=numbers[mid]) < toFind) {
                    low = (short)(mid + 1);
                } else if (number_mid > toFind) {
                    high = (short)(mid - 1);
                } else {
                    break; // Found. Would return from here in a normal search, but for this benchmark we just want to try many numbers.
                }

                // The number_mid optimisation helps AOT, but hurts the interpreter. Apparently array access is cheap compared to accessing locals in the interpreter?
                // if (numbers[mid] < toFind) {
                //     low = mid + 1;
                // } else if (numbers[mid] > toFind) {
                //     high = mid - 1;
                // } else {
                //     break; // Found. Would return from here in a normal search, but for this benchmark we just want to try many numbers.
                // }
            }
        }

        Stopwatch.measure();
    }
}
