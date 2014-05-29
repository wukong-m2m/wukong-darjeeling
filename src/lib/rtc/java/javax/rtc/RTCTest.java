package javax.rtc;

public class RTCTest {
	public static short AddFortyTwo(short x) {
		if (x==0)
			return (short)42;
		else
			return (short)43;
	}
	public static native short GetFortyThree();
	// public static short Add(short a, short b, short c, short d, short e) {
	// 	return (short)(a+b+c+d+e);
	// }
}
