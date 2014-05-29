package javax.rtc;

public class RTCTest {
	private static short test;
	private static short test_static;

	public static short test_static_short(short x) {
		test_static += x;
		return test_static;
	}

	public static short compare_short_EQ(short x) {
		if (x == 0)
			return (short)1;
		else
			return (short)0;
	}

	public static short compare_short_NE(short x) {
		if (x != 0)
			return (short)1;
		else
			return (short)0;
	}

	public static short compare_short_LT(short x) {
		if (x < 0)
			return (short)1;
		else
			return (short)0;
	}

	public static short compare_short_LE(short x) {
		if (x <= 0)
			return (short)1;
		else
			return (short)0;
	}

	public static short compare_short_GT(short x) {
		if (x > 0)
			return (short)1;
		else
			return (short)0;
	}

	public static short compare_short_GE(short x) {
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
