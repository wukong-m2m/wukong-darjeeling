/**
 * 
 */
package com.ihp;

/**
 * @author Michael Maaser
 *
 */
public class TowersOfHanoi implements BenchmarkImplementation {

	private static short[] left = new short[16];
	private static short[] middle = new short[16];
	private static short[] right = new short[16];

	public TowersOfHanoi(short towerheight) {
		left = new short[towerheight];
		middle = new short[towerheight];
		right = new short[towerheight];
		for (short i = 0; i < towerheight; i++) {
			left[i] = (short) (towerheight - i);
		}
	}

	/* (non-Javadoc)
	 * @see com.ihp.TestImplementation#runTest(int)
	 */
	public void runTest(int times) {
		for (;times>1;times-=2) {
			towersOfHanoi(left, right, middle, (short) left.length);
			towersOfHanoi(right, left, middle, (short) left.length);
		}
		if (times >0) {
			towersOfHanoi(left, right, middle, (short) left.length);
			left = right;
			right = new short[left.length];
		}
	}

	/* (non-Javadoc)
	 * @see com.ihp.TestImplementation#getName()
	 */
	public String getName() {
		return "Towers of Hanoi ("+left.length+")";
	}

	private static void towersOfHanoi(short[] from, short[] to, short[] help, short count) {
		if (count > 1) {
			towersOfHanoi(from, help, to, (short) (count-1));
		}
//		dump(left);
//		dump(middle);
//		dump(right);
//		System.out.println("");
		moveTopMost(from, to);
		if (count > 1) {
			towersOfHanoi(help, to, from, (short) (count-1));
		}
		
	}

	private static void moveTopMost(short[] from, short[] to) {
			short i = findTopMost(from);
			short j = findTopMost(to);
			to[j+1] = from[i];
			from[i] = 0;
	//		dump(left);
	//		dump(middle);
	//		dump(right);
	//		System.out.println("");
		}

	private static short findTopMost(short[] stack) {
		for (short i = (short) (stack.length - 1); i>=0; i--) {
			if (stack[i] != 0) {
				return i;
			}
		}
		return -1;
	}

}
