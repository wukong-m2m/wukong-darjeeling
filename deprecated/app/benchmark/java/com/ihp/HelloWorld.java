package com.ihp;

/**
 * A typical HelloWorld Program, framed by a measurement bracket.
 * 
 * @author Oliver Maye
 * @version 1.0
 */
public class HelloWorld implements BenchmarkImplementation {

	public void runTest(int times) {
		for (int cnt = 0; cnt < times; cnt++) {
			System.out.println("Hello World!");
		}
	}

	public String getName() {
		return "HelloWorld";
	}

//	public static void main(String[] args) {
//		BenchmarkImplementation test = new HelloWorld();
//		test.runTest(1);
//	}
	
}
