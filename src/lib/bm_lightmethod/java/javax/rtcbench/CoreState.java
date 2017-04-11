package javax.rtcbench;

public class CoreState {
	public static native boolean ee_isdigit_lightweight(byte c);
	// public static boolean ee_isdigit(byte c) {
	// 	boolean retval;
	// 	retval = ((c>='0') & (c<='9')) ? true : false;
	// 	return retval;
	// }
}