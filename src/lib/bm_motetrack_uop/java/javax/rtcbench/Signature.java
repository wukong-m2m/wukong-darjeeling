package javax.rtcbench;

import javax.rtc.RTC;

public class Signature {
    public static final byte NBR_RFSIGNALS_IN_SIGNATURE = 18;
    private static short GlobUniqSignatureID = 0;

    public short id;
    public RFSignal[] rfSignals;

    public Signature()
    {
        rfSignals = new RFSignal[NBR_RFSIGNALS_IN_SIGNATURE];
        for(short i = 0; i < NBR_RFSIGNALS_IN_SIGNATURE; ++i) {
            this.rfSignals[i] = new RFSignal();
        }
        init(this);
    }

    public static void init(Signature sig) {
        for(short i = 0; i < NBR_RFSIGNALS_IN_SIGNATURE; ++i) {
            RFSignal.init(sig.rfSignals[i]);
        }
                     
        sig.id = ++GlobUniqSignatureID;
    }
}
