package javax.darjeeling;

public class Stopwatch {
	public static native void resetAndStart();
	public static native void measure();
	public static native void setTimerNumber(byte number);
}
