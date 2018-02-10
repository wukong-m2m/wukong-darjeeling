package javax.rtcbench;

import javax.rtc.Lightweight;
import javax.rtc.RTC;

public class RTCBenchmark {
    private final static short NUMNUMBERS = 256;

    public static String name = "HEAPSORT 8 OPTIMISED";
    public static native void test_native();
    public static boolean test_java() {
        byte numbers[] = new byte[NUMNUMBERS]; // Not including this in the timing since we can't do it in C

        // Fill the array
        for (int i=0; i<NUMNUMBERS; i++)
            numbers[i] = (byte)(NUMNUMBERS - 128 - i);

        // Then sort it
        rtcbenchmark_measure_java_performance(numbers);

        for (int k=0; k<NUMNUMBERS; k++) {
            if (numbers[k] != (k - 128)) {
                return false;
            }
        }

        return true;
    }

    @Lightweight
    public static void siftDown(byte a[], short start, short end) {
        short root = 0;
        short child;
        while ( (child = (short)((root << 1)+1)) < end ) {
            short child_plus_one = (short)(child + 1);
            if ((child_plus_one < end) && (a[child] < a[child_plus_one])) {
                child += 1;
            }
            byte a_root = a[root];
            byte a_child = a[child];
            if (a_root < a_child) {
                // SWAP( a[child], a[root] );
                a[root] = a_child;
                a[child] = a_root;

                root = child;
            }
            else
                break;
        }        
    }

    public static void rtcbenchmark_measure_java_performance(byte[] a) {
        // Exact copy of http://rosettacode.org/wiki/Sorting_algorithms/Heapsort#C, but with SWAP and siftDown inlined to prevent expensive method calls

        short count = (short)a.length;
        short start, end;
     
        /* heapify */
        for (start = (short)((count-2)/2); start >=0; start--) {
            siftDown( a, start, count);
        }
     
        for (end=(short)(count-1); end > 0; end--) {
            // SWAP(a[end],a[0]);
            {byte t=a[end]; a[end]=a[0]; a[0]=t; }
            siftDown(a, (short)0, end);
        }
    }
}
