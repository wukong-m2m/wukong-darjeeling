/**
 * 
 */
package com.ihp;

/**
 * @author Michael Maaser
 *
 */
public class PrepareTestFieldOnly extends Sort {

	/* (non-Javadoc)
	 * @see com.ihp.TestImplementation#runTest()
	 */
	public void runTest(int times) {
		for (; times > 0; times--) {
			prepareSortTestField();
		}
	}

	/* (non-Javadoc)
	 * @see com.ihp.TestImplementation#getName()
	 */
	public String getName() {
		return "PrepareTestFieldOnly";
	}
}
