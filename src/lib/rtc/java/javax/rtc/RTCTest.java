package javax.rtc;

public class RTCTest {
	public static short f_LT(short x) {
		if (x < 0)
			return (short)1;
		else
			return (short)0;
	}

	public static short f_LE(short x) {
		if (x <= 0)
			return (short)1;
		else
			return (short)0;
	}

	public static short f_GT(short x) {
		if (x > 0)
			return (short)1;
		else
			return (short)0;
	}

	public static short f_GE(short x) {
		if (x >= 0)
			return (short)1;
		else
			return (short)0;
	}

	public static native short GetFortyThree();
	// public static short Add(short a, short b, short c, short d, short e) {
	// 	return (short)(a+b+c+d+e);
	// }
}
