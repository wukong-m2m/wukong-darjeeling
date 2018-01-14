package javax.rtcbench;

import javax.rtc.RTC;

public class RTCBenchmark {
    private final static short NUMNUMBERS = 256;

    public static String name = "MOTETRACK ORI";
    public static native void test_native();
    public static boolean test_java() {
    	MobileMoteM.motetrack_init_benchmark();
    	Point p = rtcbenchmark_measure_java_performance();

		RTC.avroraPrintShort(p.x);
		RTC.avroraPrintShort(p.y);
		RTC.avroraPrintShort(p.z);

        return (p.x == 210 && p.y == 249 && p.z == 0);
    }
    
    public static Point rtcbenchmark_measure_java_performance() {
    	return MobileMoteM.estLocAndSend();
  //   	NewSignature s = NewSignature.getNewSignature((short)8);
    	    
		// RTC.avroraPrintShort(s.x);
		// RTC.avroraPrintShort(s.y);
		// RTC.avroraPrintShort(s.z);
		// RTC.avroraPrintShort(s.sig_id);
		// for (int i=0; i<Signature.NBR_RFSIGNALS_IN_SIGNATURE; i++) {
		// 	RTC.beep(i);
		// 	RTC.avroraPrintShort(s.signals_source_ID[i]);
		// 	RTC.avroraPrintShort(s.signals_rssi1[i]);
		// 	RTC.avroraPrintShort(s.signals_rssi2[i]);
		// }

  //       RefSignature r = DB.getRefSignature((short)8);
		// RTC.avroraPrintShort(r.location.x);
		// RTC.avroraPrintShort(r.location.y);
		// RTC.avroraPrintShort(r.location.z);
		// RTC.avroraPrintShort(r.sig.id);
		// for (int i=0; i<Signature.NBR_RFSIGNALS_IN_SIGNATURE; i++) {
		// 	RTC.beep(i);
		// 	RTC.avroraPrintShort(r.sig.rfSignals[i].sourceID);
		// 	RTC.avroraPrintShort(r.sig.rfSignals[i].rssi[0]);
		// 	RTC.avroraPrintShort(r.sig.rfSignals[i].rssi[1]);
		// }
    }
}
