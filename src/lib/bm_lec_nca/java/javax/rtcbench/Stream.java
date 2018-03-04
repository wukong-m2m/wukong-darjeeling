package javax.rtcbench;

import javax.rtc.RTC;

public class Stream {
	public Stream(short capacity) {
		data = new byte[capacity];
		current_byte_index = 0;
		bits_used_in_current_byte = 0;
	}

	public byte[] data;
	public short current_byte_index;
	public byte bits_used_in_current_byte;
}
