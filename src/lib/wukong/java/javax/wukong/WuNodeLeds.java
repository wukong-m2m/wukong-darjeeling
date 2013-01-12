package javax.wukong;

public class WuNodeLeds
{
	public static final byte LED1 = 0;
	public static final byte LED2 = 1;
	public static final byte LED3 = 2;
	public static final byte LED4 = 3;

    public static native void _setLed(int led, boolean state);
    private static native void _init();

	static
	{
		_init();
	}

	public static void setLed(int led, boolean state)
	{
		_setLed(led, state);
	}
}
