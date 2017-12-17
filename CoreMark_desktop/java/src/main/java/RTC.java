package javax.rtc;

public class RTC
{
	public static void useRTC(boolean onOff) {}
	public static void avroraBreak() {}
    public static void avroraPrintShort(short value) {}
    public static void avroraPrintInt(int value) {}
    public static void avroraPrintHex16(short value) {}
    public static void avroraPrintHex32(int value) {}
    public static void avroraPrintSP() {}
    public static void avroraStartCountingCalls() {}
    public static void avroraStopCountingCalls() {}
	public static void beep(int number) {}
	public static void terminateOnException(short type) {}

    // For CoreMark
    private static long CorePortMe_start;
    private static long CorePortMe_stop;

    /* Function : start_time
        This function will be called right before starting the timed portion of the benchmark.

        Implementation may be capturing a system timer (as implemented in the example code) 
        or zeroing some system parameters - e.g. setting the cpu clocks cycles to 0.
    */
    public static void coremark_start_time() {
        CorePortMe_start = System.currentTimeMillis();
    }

    /* Function : stop_time
        This function will be called right after ending the timed portion of the benchmark.

        Implementation may be capturing a system timer (as implemented in the example code) 
        or other system parameters - e.g. reading the current value of cpu cycles counter.
    */
    public static void coremark_stop_time() {
        CorePortMe_stop = System.currentTimeMillis();
    }

    /* Function : get_time
        Return an abstract "ticks" number that signifies time on the system.
        
        Actual value returned may be cpu cycles, milliseconds or any other value,
        as long as it can be converted to seconds by <time_in_secs>.
        This methodology is taken to accomodate any hardware or simulated platform.
        The sample implementation returns millisecs by default, 
        and the resolution is controlled by <TIMER_RES_DIVIDER>
    */
    public static int coremark_get_time() {
        return (int)(CorePortMe_stop - CorePortMe_start);
    }
}
