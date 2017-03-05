package javax.rtc;

public class RTC
{
	public static native void useRTC(boolean onOff);
	public static native void avroraBreak();
    public static native void avroraPrintShort(short value);
    public static native void avroraPrintInt(int value);
    public static native void avroraPrintHex16(short value);
    public static native void avroraPrintHex32(int value);
    public static native void avroraPrintSP();

	public static native void beep(int number);
	public static native void terminateOnException(short type);
}
