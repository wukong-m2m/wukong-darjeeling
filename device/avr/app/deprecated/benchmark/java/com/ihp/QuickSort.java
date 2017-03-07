/**
 * 
 */
package com.ihp;

/**
 * @author Michael Maaser
 *
 */
public class QuickSort extends Sort implements BenchmarkImplementation {

	public void runTest(int times) {
		short lastElement = (short) (testfield.length-1);
		for (; times > 0; times--) {
			prepareSortTestField();
			quickSort((short) 0, lastElement, testfield);
		}
	}

	private static short teile(short links, short rechts, short[] daten) {
		short i = links;
		short j = (short) (rechts - 1);
		short pivot = daten[rechts];
	
		do {
	
			while (daten[i] <= pivot && i < rechts) {
				i++;
			}
	
			while (daten[j] >= pivot && j > links) {
				j--;
			}
	
			if (i < j) {
				short temp = daten[i];
				daten[i] = daten[j];
				daten[j] = temp;
			}
	
		} while (i < j);
	
		if (daten[i] > pivot) {
			short temp = daten[i];
			daten[i] = daten[rechts];
			daten[rechts] = temp;
		}
	
		return i;
	}

	private static void quickSort(short left, short right, short[] daten) {
		if (left < right) {
			short teiler = teile(left, right, daten);
			quickSort(left, (short) (teiler - 1), daten);
			quickSort((short) (teiler + 1), right, daten);
		}
	}
	
	public String getName() {
		// TODO Auto-generated method stub
		return "QuickSort";
	}
}
