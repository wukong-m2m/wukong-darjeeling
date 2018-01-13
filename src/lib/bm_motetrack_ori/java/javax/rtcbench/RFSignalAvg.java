package javax.rtcbench;

import javax.rtc.RTC;

public class RFSignalAvg {
	short sourceID;
	short rssiSum[];
	byte nbrSamples[];

	public RFSignalAvg() {
		rssiSum = new short[MoteTrackParams.NBR_FREQCHANNELS];
		nbrSamples = new byte[MoteTrackParams.NBR_FREQCHANNELS];
	}
}
