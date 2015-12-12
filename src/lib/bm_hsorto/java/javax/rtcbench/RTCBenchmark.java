package javax.rtcbench;

import javax.darjeeling.Stopwatch;

public class RTCBenchmark {
    private final static short NUMNUMBERS = 256;

    public static String name = "HEAPSORT OPTIMISED";
    public static native void test_native();
    public static boolean test_java() {
        short numbers[] = new short[NUMNUMBERS]; // Not including this in the timing since we can't do it in C

        // Fill the array
        for (int i=0; i<NUMNUMBERS; i++)
            numbers[i] = (short)(NUMNUMBERS - 1 - i);

        // Then sort it
        rtcbenchmark_measure_java_performance(numbers);

        for (int k=0; k<NUMNUMBERS; k++) {
            if (numbers[k] != k) {
                return false;
            }
        }

        return true;
    }

    public static void rtcbenchmark_measure_java_performance(short[] a) {
        Stopwatch.resetAndStart();

        // Exact copy of http://rosettacode.org/wiki/Sorting_algorithms/Heapsort#C, but with SWAP and siftDown inlined to prevent expensive method calls

        int count = a.length;
        int start, end;
     
        /* heapify */
        for (start = (count-2)/2; start >=0; start--) {
            // siftDown( a, start, count);
            // void siftDown( short *a, int start, int count)
            // {
            int root = start;
            int child;
            while ( (child = root*2+1) < count ) {
                int child_plus_one = child + 1;
                if ((child_plus_one < count) && (a[child] < a[child_plus_one])) {
                    child += 1;
                }
                short a_root = a[root];
                short a_child = a[child];
                if (a_root < a_child) {
                    // SWAP( a[child], a[root] );
                    a[root] = a_child;
                    a[child] = a_root;

                    root = child;
                }
                else
                    break;
            }
            // }
        }
     
        for (end=count-1; end > 0; end--) {
            // SWAP(a[end],a[0]);
            {short t=a[end]; a[end]=a[0]; a[0]=t; }

            // siftDown(a, 0, end);
            // void siftDown( short *a, int start, int end)
            // {
            int root = 0;
            int child;
            while ( (child = root*2+1) < end ) {
                int child_plus_one = child + 1;
                if ((child_plus_one < end) && (a[child] < a[child_plus_one])) {
                    child += 1;
                }
                short a_root = a[root];
                short a_child = a[child];
                if (a_root < a_child) {
                    // SWAP( a[child], a[root] );
                    a[root] = a_child;
                    a[child] = a_root;

                    root = child;
                }
                else
                    break;
            }
            // }
        }
        
        // void siftDown( short *a, int start, int end)
        // {
        //     int root = start;
         
        //     while ( root*2+1 < end ) {
        //         int child = 2*root + 1;
        //         if ((child + 1 < end) && (a[child] < a[child+1])) {
        //             child += 1;
        //         }
        //         if (a[root] < a[child]) {
        //             // SWAP( a[child], a[root] );
        //             {short t=a[child]; a[child]=a[root]; a[root]=t; }

        //             root = child;
        //         }
        //         else
        //             return;
        //     }
        // }
        Stopwatch.measure();
    }
}
