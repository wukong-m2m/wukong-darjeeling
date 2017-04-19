package javax.rtcbench;

import javax.rtc.Lightweight;

public class CoreUtil {
	@Lightweight
	static short crc(int intdata, short crc, short number_of_bytes )
	{
		while (number_of_bytes > 0) {
			short data = (short)intdata;

			// for (short i = 0; i < 8; i++)
			for (short i = (short)-8; i != 0; i++) // This is faster because a !=0 check is faster than <8.
		    {
				if (((data ^ crc) & 1) == 0)
				{
					crc >>>= 1;
					crc &= (short)0x7fff;
				}
				else
				{
					crc ^= (short)0x4002;
					crc >>>= 1;
					crc |= (short)-0x8000;
				}
				data >>>= 1;
		    }

		    number_of_bytes--;
		    intdata >>>= 8;
		}
		return crc;
	} 
}
