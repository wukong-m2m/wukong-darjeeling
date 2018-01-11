package javax.rtcbench;

import javax.rtc.RTC;

public class RTCBenchmark {
    private final static short NUMNUMBERS = 256;

    public static String name = "MOTETRACK";
    public static native void test_native();
    public static boolean test_java() {
    	rtcbenchmark_measure_java_performance();
        return true;
    }
    
    public static void rtcbenchmark_measure_java_performance() {
    	NewSignature s = NewSignature.getNewSignature((short)8);
    	    
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

        RefSignature r = NewSignature.getRefSignature((short)8);
		RTC.avroraPrintShort(r.location.x);
		RTC.avroraPrintShort(r.location.y);
		RTC.avroraPrintShort(r.location.z);
		RTC.avroraPrintShort(r.sig.id);
		for (int i=0; i<NewSignature.NBR_RFSIGNALS_IN_SIGNATURE; i++) {
			RTC.beep(i);
			RTC.avroraPrintShort(r.sig.rfSignals[i].sourceID);
			RTC.avroraPrintShort(r.sig.rfSignals[i].rssi[0]);
			RTC.avroraPrintShort(r.sig.rfSignals[i].rssi[1]);
		}
    }
}
