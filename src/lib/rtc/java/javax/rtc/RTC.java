package javax.rtc;

public class RTC
{
	public static native void useRTC(boolean onOff);
	public static native void avroraBreak();
    public static native void avroraPrintShort(short value);
    public static native void avroraPrintInt(int value);
    public static native void avroraPrintSP();
}
