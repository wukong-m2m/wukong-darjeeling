package javax.rtcbench;

import javax.rtc.RTC;

public class RFSignalAvg {
	short sourceID;
	short rssiSum_0;
	byte nbrSamples_0;
	short rssiSum_1;
	byte nbrSamples_1;

	public RFSignalAvg() {
	}
	public static void init(RFSignalAvg rfSigPtr) {
		rfSigPtr.sourceID = 0;
		rfSigPtr.rssiSum_0 = 0;
		rfSigPtr.nbrSamples_0 = 0;
		rfSigPtr.rssiSum_1 = 0;
		rfSigPtr.nbrSamples_1 = 0;		
	}
}
