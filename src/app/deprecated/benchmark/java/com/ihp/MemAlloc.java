/**
 * 
 */
package com.ihp;

/**
 * @author Michael Maaser
 *
 */
public class MemAlloc implements BenchmarkImplementation {

	private static final int NR_OF_MEM_BLOCKS = 50;

//	public static void main(String[] args) {
//		BenchmarkImplementation test = new MemAlloc();
//		test.runTest(1);
//	}
	
	/* (non-Javadoc)
	 * @see com.ihp.TestImplementation#runTest(int)
	 */
	public void runTest(int times) {
		for (;times > 0; times--) {
			Object[] v = new Object[NR_OF_MEM_BLOCKS];
			for (int i = 0; i < NR_OF_MEM_BLOCKS; i++) {
				v[i]=new byte[100];
				sleep(100);
			}
			for (int i = NR_OF_MEM_BLOCKS-1; i >= 0; i-=2) {
				v[i]=null;
				sleep(100);
			}
			for (int i = 1; i < NR_OF_MEM_BLOCKS; i+=2) {
				v[i]=new byte[100];
				sleep(100);
			}
			v = null;
			sleep(1000);
		}
	}

	private void dumpMem() {
		System.out.println("Remaining Free Mem:"+Runtime.getRuntime().freeMemory());
	}
	
	private void sleep(int ms) {
//		dumpMem();
		try {
			Thread.sleep(ms);
		} catch (Exception e) {
		}
	}

	/* (non-Javadoc)
	 * @see com.ihp.TestImplementation#getName()
	 */
	public String getName() {
		return "Memory Allocationa Garbage Collection Test";
	}

}
