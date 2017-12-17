package javax.rtcbench;

import javax.rtc.RTC;

public class RTCBenchmark {
    private final static short NUMNUMBERS = 256;

    public static String name = "HEAPSORT OPTIMISED";
    public static native void test_native();
    public static boolean test_java() {
        int numbers[] = new int[NUMNUMBERS]; // Not including this in the timing since we can't do it in C

        // Fill the array
        for (int i=0; i<NUMNUMBERS; i++)
            numbers[i] = (NUMNUMBERS - 1 - i);

        // Then sort it
        rtcbenchmark_measure_java_performance(numbers);

        for (int k=0; k<NUMNUMBERS; k++) {
            if (numbers[k] != k) {
                return false;
            }
        }

        return true;
    }

    public static void rtcbenchmark_measure_java_performance(int[] a) {
        // Exact copy of http://rosettacode.org/wiki/Sorting_algorithms/Heapsort#C, but with SWAP and siftDown inlined to prevent expensive method calls

        short count = (short)a.length;
        short start, end;
     
        /* heapify */
        for (start = (short)((count-2)/2); start >=0; start--) {
            siftDown( a, start, count);
        }
     
        for (end=(short)(count-1); end > 0; end--) {
            // SWAP(a[end],a[0]);
            {int t=a[end]; a[end]=a[0]; a[0]=t; }
            siftDown(a, (short)0, end);
        }
    }

    public static void siftDown(int a[], short start, short end) {
        short root = 0;
        short child;
        while ( (child = (short)((root << 1)+1)) < end ) {
            short child_plus_one = (short)(child + 1);
            if ((child_plus_one < end) && (a[child] < a[child_plus_one])) {
                child += 1;
            }
            int a_root = a[root];
            int a_child = a[child];
            if (a_root < a_child) {
                // SWAP( a[child], a[root] );
                a[root] = a_child;
                a[child] = a_root;

                root = child;
            }
            else
                return;
        }        
    }
}
