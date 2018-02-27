package javax.rtcbench;

import javax.rtc.RTC;
import javax.rtc.Lightweight;

public class DB {
    public final static short REFSIGNATUREDB_SIZE = 257;
    public final static short SIGNATUREDB_SIZE = 1; // commented out the other signature since we only use the first one
    // public final static short SIGNATUREDB_SIZE = 74;

    @Lightweight
    public static void signature_get(RefSignature s, short index) {
        Point location = s.location;
        Signature sig = s.sig;
        location.x = SignatureDB.X.data[index];
        location.y = SignatureDB.Y.data[index];
        location.z = SignatureDB.Z.data[index];
        sig.id = SignatureDB.SigID.data[index];

        short rfSignalIndex = (short)(index * Signature.NBR_RFSIGNALS_IN_SIGNATURE); // sourceID, rssi0 and rssi1 arrays contain the Signature.NBR_RFSIGNALS_IN_SIGNATURE elements for each RefSignature in a row.
        RFSignal[] rfSignals = sig.rfSignals;
        for (short i=0; i<Signature.NBR_RFSIGNALS_IN_SIGNATURE; i++) {
            RFSignal rfSignal = rfSignals[i];
            rfSignal.sourceID = SignatureDB.SourceID.data[rfSignalIndex];
            rfSignal.rssi_0 = SignatureDB.Rssi0.data[rfSignalIndex];
            rfSignal.rssi_1 = SignatureDB.Rssi1.data[rfSignalIndex];
            rfSignalIndex++;
        }
    }

    @Lightweight
    public static void refSignature_get(RefSignature s, short index) {
        Point location = s.location;
        Signature sig = s.sig;
        location.x = RefSignatureDB.X.data[index];
        location.y = RefSignatureDB.Y.data[index];
        location.z = RefSignatureDB.Z.data[index];
        sig.id = RefSignatureDB.SigID.data[index];

        short rfSignalIndex = (short)(index * Signature.NBR_RFSIGNALS_IN_SIGNATURE); // sourceID, rssi0 and rssi1 arrays contain the Signature.NBR_RFSIGNALS_IN_SIGNATURE elements for each RefSignature in a row.
	    RFSignal[] rfSignals = sig.rfSignals;
        for (short i=0; i<Signature.NBR_RFSIGNALS_IN_SIGNATURE; i++) {
        	RFSignal rfSignal = rfSignals[i];
        	rfSignal.sourceID = RefSignatureDB.SourceID.data[rfSignalIndex];
        	rfSignal.rssi_0 = RefSignatureDB.Rssi0.data[rfSignalIndex];
        	rfSignal.rssi_1 = RefSignatureDB.Rssi1.data[rfSignalIndex];
        	rfSignalIndex++;
        }
    }
}
