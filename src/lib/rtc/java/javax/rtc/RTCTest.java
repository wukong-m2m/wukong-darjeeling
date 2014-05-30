package javax.rtc;

public class RTCTest {
	private static short static_short1;
	private static short static_short2;

	public short instance_short1;
	public int instance_int;
	public short instance_short2;

	public short test_instance_short1(short x) {
		this.instance_short1 += x;
		return this.instance_short1;
	}

	public short test_instance_short2(short x) {
		this.instance_short2 += x;
		return this.instance_short2;
	}

	public static short test_static_short(short x) {
		static_short2 += x;
		return static_short2;
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
