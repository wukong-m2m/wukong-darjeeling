package javax.rtcbench;

import javax.rtc.RTC;

public class Signature {
    public static final byte NBR_RFSIGNALS_IN_SIGNATURE = 14;
    private static short GlobUniqSignatureID = 0;

    public short id;
    public RFSignal[] rfSignals = new RFSignal[NBR_RFSIGNALS_IN_SIGNATURE];

    public Signature()
    {
        short i = 0;

        for(i = 0; i < NBR_RFSIGNALS_IN_SIGNATURE; ++i) {
            this.rfSignals[i] = new RFSignal();
        }
                     
        this.id = ++GlobUniqSignatureID;
    }
}
