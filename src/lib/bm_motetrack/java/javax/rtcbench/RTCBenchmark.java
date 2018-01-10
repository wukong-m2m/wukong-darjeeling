package javax.rtcbench;

import javax.rtc.RTC;

public class RTCBenchmark {
    private final static short NUMNUMBERS = 256;

    public static String name = "MOTETRACK";
    public static native void test_native();
    public static boolean test_java() {
    	NewSignature s = NewSignature.getSignature((short)8);
    	    
		RTC.avroraPrintShort(s.x);
		RTC.avroraPrintShort(s.y);
		RTC.avroraPrintShort(s.z);
		RTC.avroraPrintShort(s.sig_id);
		for (int i=0; i<NewSignature.NBR_RFSIGNALS_IN_SIGNATURE; i++) {
			RTC.beep(i);
			RTC.avroraPrintShort(s.signals_source_ID[i]);
			RTC.avroraPrintShort(s.signals_rssi1[i]);
			RTC.avroraPrintShort(s.signals_rssi2[i]);
		}

        return true;
    }
    
    public static void rtcbenchmark_measure_java_performance(int[] numbers) {

    }
}
