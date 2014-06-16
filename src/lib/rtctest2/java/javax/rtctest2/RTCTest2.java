package javax.rtctest2;

// This infusion won't be RTC compiled for now.
public class RTCTest2 {
	// To test access to statics in other infusions
	public static short rtc_test_short1;
	public static int rtc_test_int1;
	public static int rtc_test_int2;
	public static short rtc_test_short2;

	// To test calls from RTC code to JVM (not compiled) code
	public static int test_method_call_7b(int a) { return test_method_call_7c(a); }
	public static int test_method_call_7c(int a) { return a+42; }	
}