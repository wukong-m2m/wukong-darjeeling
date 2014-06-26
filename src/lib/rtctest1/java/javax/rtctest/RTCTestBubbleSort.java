package javax.rtctest;

import javax.darjeeling.Stopwatch;

public class RTCTestBubbleSort {
	public static void test_bubblesort() {

		short NUMNUMBERS = 256;
		short numbers[] = new short[NUMNUMBERS]; // Not including this in the timing since we can't do it in C

		Stopwatch.resetAndStart();

		// Fill the array
		for (int i=0; i<NUMNUMBERS; i++)
			numbers[i] = (short)(NUMNUMBERS - 1 - i);

		// Then sort it
		for (short i=0; i<(short)NUMNUMBERS; i++) {
			for (short j=0; j<((short)(NUMNUMBERS-i-1)); j++) {
				if (numbers[j]>numbers[j+1]) {
					short temp = numbers[j];
					numbers[j] = numbers[j+1];
					numbers[((short)(j+1))] = temp;
				}
			}
		}

		Stopwatch.measure();
	}

	public static native void test_bubblesort_native();
}
