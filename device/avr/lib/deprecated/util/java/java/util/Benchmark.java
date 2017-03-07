package java.util;

public class Benchmark{
	public native static void start();
	public native static boolean stop(String message, int sec);
}