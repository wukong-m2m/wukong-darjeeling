package com.ihp;

public class QuickSortIterativ extends Sort implements BenchmarkImplementation {

	public static int wgroesser(int w1, int w2) {
		int groesser;
		if (w1>w2) {
			groesser=w1;
		}
		else {
			groesser=w2;
		}
		return groesser;
	}
	
	public static short[] quicksort(short menge[]) {
		if (menge==null) {
			return menge;
		}
		int stapel[]=new int[32];
		int s=0;
		int links = 0;
		int rechts = menge.length-1;
		int i;
		int j;
		int a = 0;
		while (a!=1) {
			if (links < rechts) {
				short pivot=menge[rechts-1];
				i=links-1;
				j=rechts+1;
				int b = 0;
				while (b!=1) {
					do {
						i++;
					} while (menge[i]<pivot);
					do {
						j--;
					} while (menge[j]>pivot);
					if (i>=j) {
						break;
					} else {
						short tmp=menge[i];
						menge[i]=menge[j];
						menge[j]=tmp;
					}
				}
				stapel[s]=rechts;
				s++;
				rechts=wgroesser(links, i-1);
			} else {
				if (s==0) {
					break;
				} else {
					links=rechts+1;
					s--;
					rechts=stapel[s];
				}
			}
		}
		return menge;
	}
	
	public void runTest(int times) {
		for (int a=0; a<times; a++) {
			prepareSortTestField();
			testfield = quicksort(testfield);
//			outputArray("Result testfield", testfield);
		}
	}
	
	public String getName() {
		return "QuickSortIterativ";
	}
	
//	public static void main(String[] args) {
//		Sort srt = new QuickSortIterativ();
//		srt.runTest(1);
//	}
}
