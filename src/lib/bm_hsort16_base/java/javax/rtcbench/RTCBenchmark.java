package javax.rtcbench;

import javax.rtc.Lightweight;
import javax.rtc.RTC;

public class RTCBenchmark {
    private final static short NUMNUMBERS = 256;

    public static String name = "HEAPSORT 16 BASE";
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

    @Lightweight
    private static void SWAP(short[] array, short r, short s) {
        short t=array[r];
        array[r]=array[s];
        array[s]=t;
    }

    @Lightweight
    public static void siftDown(short a[], short start, short end) {
        short root = 0;

        while ( root*2+1 < end ) {
            short child = (short)(2*root + 1);
            if ((child+1 < end) && (a[child] < a[(short)child+1])) {
                child += 1;
            }
            if (a[root] < a[child]) {
                SWAP(a, child, root);

                root = child;
            }
            else
                break;
        }        
    }

    public static void rtcbenchmark_measure_java_performance(short[] a) {
        // Exact copy of http://rosettacode.org/wiki/Sorting_algorithms/Heapsort#C

        short count = (short)a.length;
        short start, end;
     
        /* heapify */
        for (start = (short)((count-2)/2); start >=0; start--) {
            siftDown( a, start, count);
        }
     
        for (end=(short)(count-1); end > 0; end--) {
            SWAP(a, end, (short)0);
            siftDown(a, (short)0, end);
        }
    }
}
