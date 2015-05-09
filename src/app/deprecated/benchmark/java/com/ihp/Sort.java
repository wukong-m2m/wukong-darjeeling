/**
 * 
 */
package com.ihp;


/**
 * @author Michael Maaser
 *
 */
public class Sort implements BenchmarkImplementation {

	protected short[] testfield = new short[(short) 253];

	protected void outputArray(String prefix, short field[]) {
		if( prefix!=null ) {
			System.out.println(prefix);
		}
		if (field==null) {
			System.out.println("Array ist null");
		}
		else {
			System.out.print("Array length="+field.length+":  ");
			for ( int a=0 ; a<field.length ; a++) {
				System.out.print(field[a] + ", ");
			}
			System.out.println();
			System.out.println();
		}
	}
	
	private short modpow(int i, int j, int length) {
		short result = 1;
		for (;j>0;j--) {
			result = (short) ((result * i) % length);
		}
		return result;
	}

	/* (non-Javadoc)
	 * @see com.ihp.TestImplementation#runTest(int)
	 */
	public void runTest(int times) {
		for (; times > 0; times--) {
			prepareSortTestField();
		}
	}

	protected void prepareSortTestField() {
		for (int i = 0; i < testfield.length; i++) {
			testfield[i] = modpow(i, 17, testfield.length);
		}
	}

	/* (non-Javadoc)
	 * @see com.ihp.TestImplementation#getName()
	 */
	public String getName() {
		return "SortPreparation";
	}

}
