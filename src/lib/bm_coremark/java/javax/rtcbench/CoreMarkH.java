package javax.rtcbench;
import javax.rtc.RTC;

public class CoreMarkH {
	public static short TOTAL_DATA_SIZE		= 2*1000;

	/* Algorithm IDS */
	public static byte ID_LIST				= (1<<0);
	public static byte ID_MATRIX 			= (1<<1);
	public static byte ID_STATE 			= (1<<2);
	public static byte ALL_ALGORITHMS_MASK	= (byte)(ID_LIST|ID_MATRIX|ID_STATE);
	public static byte NUM_ALGORITHMS		= 3;
}