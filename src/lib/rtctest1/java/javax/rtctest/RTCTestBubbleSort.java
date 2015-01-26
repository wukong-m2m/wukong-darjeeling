package javax.rtctest;

import javax.darjeeling.Stopwatch;

public class RTCTestBubbleSort {
	public class pair { public short a; public short b; }

	private final static short NUMNUMBERS = 256;

	public static native void test_bubblesort_native();
	public static void test_bubblesort() {
		short numbers[] = new short[NUMNUMBERS]; // Not including this in the timing since we can't do it in C

		// Fill the array
		for (int i=0; i<NUMNUMBERS; i++)
			numbers[i] = (short)(NUMNUMBERS - 1 - i);

		// Then sort it
		do_bubblesort(numbers);
	}

	public static void do_bubblesort(short[] numbers) {
		Stopwatch.resetAndStart();


		for (short i=0; i<(short)NUMNUMBERS; i++) {
			short x=(short)(NUMNUMBERS-i-1); // This doesn't get optimised the way I expected it would. Without this extra variable, it will calculate NUMNUMBERS-i-1 on each interation of the inner loop! (speedup 14.7M -> 14.2M cycles)
			for (short j=0; j<x; j++) {
				if (numbers[j]>numbers[j+1]) {
					short temp = numbers[j];
					numbers[j] = numbers[j+1];
					numbers[((short)(j+1))] = temp;
				}
			}
		}

		Stopwatch.measure();
	}

	public static void swap(pair obj) {
		short tmp = obj.a;
		obj.a = obj.b;
		obj.b = tmp;
	}
}
