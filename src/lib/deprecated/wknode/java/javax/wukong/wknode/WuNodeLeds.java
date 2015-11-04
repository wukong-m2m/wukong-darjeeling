package javax.wukong.wknode;

public class WuNodeLeds
{
	public static final byte LED1 = 0;
	public static final byte LED2 = 1;
	public static final byte LED3 = 2;
	public static final byte LED4 = 3;

    public static native void setLed(int led, boolean state);
}
