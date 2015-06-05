/**
 * 
 */
package com.ihp;

/**
 * @author Michael Maaser
 *
 */
public class CalculationInLoop implements BenchmarkImplementation {

	/* (non-Javadoc)
	 * @see com.ihp.TestImplementation#runTest(int)
	 */
	public void runTest(int times) {
		for (;times>0;times--){
			int a = 1;
			for (int i = 1; i < 70; i++) {
				a*=i;
			}
		}
	}

	/* (non-Javadoc)
	 * @see com.ihp.TestImplementation#getName()
	 */
	public String getName() {
		return "Calculation 69! in a loop";
	}

//	public static void main(String[] args) {
//		BenchmarkImplementation test = new CalculationInLoop();
//		test.runTest(1);
//	}
	
}
