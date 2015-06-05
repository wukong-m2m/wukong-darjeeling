/**
 * 
 */
package com.ihp;

/**
 * @author Michael Maaser
 *
 */
public class CalculationUnrolled implements BenchmarkImplementation {

	/* (non-Javadoc)
	 * @see com.ihp.TestImplementation#runTest(int)
	 */
	public void runTest(int times) {
		for (;times>0;times--){
			int a = 1;
			a *=1;
			a *=2;
			a *=3;
			a *=4;
			a *=5;
			a *=6;
			a *=7;
			a *=8;
			a *=9;
			a *=10;
			a *=11;
			a *=12;
			a *=13;
			a *=14;
			a *=15;
			a *=16;
			a *=17;
			a *=18;
			a *=19;
			a *=20;
			a *=21;
			a *=22;
			a *=23;
			a *=24;
			a *=25;
			a *=26;
			a *=27;
			a *=28;
			a *=29;
			a *=30;
			a *=31;
			a *=32;
			a *=33;
			a *=34;
			a *=35;
			a *=36;
			a *=37;
			a *=38;
			a *=39;
			a *=40;
			a *=41;
			a *=42;
			a *=43;
			a *=44;
			a *=45;
			a *=46;
			a *=47;
			a *=48;
			a *=49;
			a *=50;
			a *=51;
			a *=52;
			a *=53;
			a *=54;
			a *=55;
			a *=56;
			a *=57;
			a *=58;
			a *=59;
			a *=60;
			a *=61;
			a *=62;
			a *=63;
			a *=64;
			a *=65;
			a *=66;
			a *=67;
			a *=68;
			a *=69;
		}
	}

	/* (non-Javadoc)
	 * @see com.ihp.TestImplementation#getName()
	 */
	public String getName() {
		return "Calculation 69! unrolled";
	}

//	public static void main(String[] args) {
//		BenchmarkImplementation test = new CalculationUnrolled();
//		test.runTest(1);
//	}
	
}
