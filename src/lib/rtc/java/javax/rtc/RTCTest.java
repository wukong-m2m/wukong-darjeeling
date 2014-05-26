package javax.rtc;

public class RTCTest {
	public static short AddFortyTwo(short x) {
		return (short)(x%42);
	}
	public static native short GetFortyThree();
	public static short Add(short a, short b, short c, short d, short e) {
		return (short)(a+b+c+d+e);
	}

}
