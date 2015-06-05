/**
 * 
 */
package com.ihp;


/**
 * @author Michael Maaser
 * 
 */
public class Benchmark {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		System.out.println();
		System.out.println("Total RAM :" + Runtime.getRuntime().totalMemory());
		// Hello World!
		test(new HelloWorld());

		// Sorting
		test(new Sort());// this prepares only the testfield
		test(new BubbleSort());
		test(new QuickSort());
		test(new QuickSortIterativ());

		// Memory and garbage collection
		test(new MemAlloc());

		// Arithmetics
		test(new CalculationInLoop());
		test(new CalculationUnrolled());

		// Towers of Hanoi
		test(new TowersOfHanoi((short) 10));
		
		// Additional stuff
		test(new Ackermann());
//		test(new DhryStone());
//		test(new WhetStoneFloat());
//		test(new WhetStoneDouble());

		System.out.println("All tests done.");
		System.out.println();
	}
	
	private static void test(BenchmarkImplementation timpl) {
		System.out.println("Test Routine: " + timpl.getName() );
		System.out.println("================================================" );
		System.out.println(" Runs | Meas.Interval[ms] | Time[ms] | MinMem | MaxMem | DiffMem");
		System.out.println("------+-------------------+----------+--------+--------+--------");
		for (int i = 0; i < 3; i++) {
			int j = 1 << i;
			benchmark(timpl, j, 100);
			benchmark(timpl, j, 50);
			benchmark(timpl, j, 0);
		}
		System.out.println();
	}

	private static void benchmark(BenchmarkImplementation timpl, int j, int memSampleRate) {
		long totalMemory=0;
		MemoryWatcher memoryWatcher=null;
		long maxUsed=0;
		long minUsed=0;
		
		System.gc();
		try {
			Thread.sleep(3000);
		} catch(Exception e) {};

		if( memSampleRate>0 ) {
			totalMemory = Runtime.getRuntime().totalMemory();
			memoryWatcher = startMemoryWatcher(memSampleRate);
			memoryWatcher.explicitGC();
			memoryWatcher.reset();
		}
		
		long start = System.currentTimeMillis();
		timpl.runTest(j);
		long end = System.currentTimeMillis();
		
		if( memSampleRate>0 ) {
			memoryWatcher.measure();
			memoryWatcher.explicitGC();
			memoryWatcher.measure();
			maxUsed = totalMemory-memoryWatcher.getMinFree();
			minUsed = totalMemory-memoryWatcher.getMaxFree();
		}
		
		System.out.print("  "+j+"   |         "+memSampleRate+"        |    " +
						  ((end - start)/j) );
		if( memSampleRate>0 ) {
			System.out.print("  |  " + minUsed + "  |  " + maxUsed + "  |  "+(maxUsed-minUsed));
			memoryWatcher.cancel();
			memoryWatcher = null;
		}
		System.out.println();
	}
	
	private static MemoryWatcher startMemoryWatcher(int msBetweenSamples) {
		MemoryWatcher mw = new MemoryWatcher(msBetweenSamples);
		if (msBetweenSamples > 0) {
			mw.start();
		}
		return mw;
	}

}
