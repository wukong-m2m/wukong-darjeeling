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
    public static native void avroraStartCountingCalls();
    public static native void avroraStopCountingCalls();
	public static native void beep(int number);
	public static native void terminateOnException(short type);
    
    public static native void startBenchmarkMeasurement_Native(); 
    public static native void startBenchmarkMeasurement_AOT(); 
    public static native void stopBenchmarkMeasurement(); 

    // For CoreMark
    /* Function : start_time
        This function will be called right before starting the timed portion of the benchmark.

        Implementation may be capturing a system timer (as implemented in the example code) 
        or zeroing some system parameters - e.g. setting the cpu clocks cycles to 0.
    */
    public static void coremark_start_time() { coremark_start_time_nat(); }
    public static native void coremark_start_time_nat();

    /* Function : stop_time
        This function will be called right after ending the timed portion of the benchmark.

        Implementation may be capturing a system timer (as implemented in the example code) 
        or other system parameters - e.g. reading the current value of cpu cycles counter.
    */
    public static native void coremark_stop_time();

    /* Function : get_time
        Return an abstract "ticks" number that signifies time on the system.
        
        Actual value returned may be cpu cycles, milliseconds or any other value,
        as long as it can be converted to seconds by <time_in_secs>.
        This methodology is taken to accomodate any hardware or simulated platform.
        The sample implementation returns millisecs by default, 
        and the resolution is controlled by <TIMER_RES_DIVIDER>
    */
    public static native int coremark_get_time();
}
