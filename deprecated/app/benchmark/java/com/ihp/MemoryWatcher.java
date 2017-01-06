/**
 * 
 */
package com.ihp;

/**
 * @author Michael Maaser
 *
 */
public class MemoryWatcher extends Thread {

	private boolean stopped = false;
	private Runtime myRuntime;
	private int maxFree;
	private int minFree;
	private int base;
	private Object lock;
	private int msBetweenSamples;

	public MemoryWatcher(int msBetweenSamples) {
		myRuntime = Runtime.getRuntime();
		this.msBetweenSamples = msBetweenSamples;
		lock = new Object();
		reset();
	}
	
	public void run() {
		while (!stopped ) {
			measure();
			try {
				Thread.sleep(msBetweenSamples);
			} catch (Exception e) {
			}
		}
	}
	
	public void explicitGC() {
		myRuntime.gc();
		try {
			Thread.sleep(1000);
		} catch (Exception e) {
		}
	}
	
	public void measure() {
		synchronized(lock) {
			int free = (int) myRuntime.freeMemory();
			maxFree = Math.max(maxFree, free);
			minFree = Math.min(minFree , free);
		}
	}
	
	public void reset() {
		synchronized(lock) {
			base = (int) myRuntime.freeMemory();
//			System.out.println("Base FreeMem:"+base);
			maxFree = base;
			minFree = base;
		}
	}

	public long getMinFree() {
		return minFree;
	}

	public long getMaxFree() {
		return maxFree;
	}
	
	public long getMinUsed() {
		return (base-maxFree);
	}
	
	public long getMaxUsed() {
		return (base-minFree);
	}

	public void cancel() {
		stopped = true;
	}
}
