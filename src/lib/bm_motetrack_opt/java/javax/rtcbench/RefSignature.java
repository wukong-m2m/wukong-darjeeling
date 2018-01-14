package javax.rtcbench;

import javax.rtc.RTC;

public class RefSignature {
    public Point location;
    public Signature sig;

    public RefSignature() {
        this.location = new Point();
        this.sig = new Signature();
    }

    public static void signatureDiffBidirectional(short results[], RefSignature refSigPtr, Signature sigPtr)
    {
        short s = 0, r = 0, f = 0; //, p = 0;
        short[] currSigDiffs = new short[MoteTrackParams.NBR_FREQCHANNELS]; // [NBR_TXPOWERS];

        // (1) - Initialize the results
        for (f = 0; f < MoteTrackParams.NBR_FREQCHANNELS; ++f)
            results[f] = 0;

        // (2) - Compute differences, while there are more RFSignals in
        //       either the Signature or the RefSignature
        while( (s < Signature.NBR_RFSIGNALS_IN_SIGNATURE && sigPtr.rfSignals[s].sourceID != 0) ||
               (r < Signature.NBR_RFSIGNALS_IN_SIGNATURE && refSigPtr.sig.rfSignals[r].sourceID != 0) ) {

            // case 1: there are no more rfSignals in Signature
            if ( !(s < Signature.NBR_RFSIGNALS_IN_SIGNATURE && sigPtr.rfSignals[s].sourceID != 0) ) {
                RFSignal.rfSignalDiff(currSigDiffs, refSigPtr.sig.rfSignals[r++], null);
            }
            // case 2: there are no more rfSignals in RefSignature
            else if ( !(r < Signature.NBR_RFSIGNALS_IN_SIGNATURE && refSigPtr.sig.rfSignals[r].sourceID != 0) ) {
                RFSignal.rfSignalDiff(currSigDiffs, sigPtr.rfSignals[s++], null);
            }

            // If we made it this far, then there are more rfSignals in both Signature and RefSignature
            // case 3: there is a match
            else if (sigPtr.rfSignals[s].sourceID == refSigPtr.sig.rfSignals[r].sourceID) {
                RFSignal.rfSignalDiff(currSigDiffs, sigPtr.rfSignals[s++], refSigPtr.sig.rfSignals[r++] );
            }
            // case 4: rfSignal of Signature < rfSignal of RefSignature
            else if (sigPtr.rfSignals[s].sourceID < refSigPtr.sig.rfSignals[r].sourceID) {
                RFSignal.rfSignalDiff(currSigDiffs, sigPtr.rfSignals[s++], null);
            }
            // case 5: rfSignal of Signature > rfSignal of RefSignature
            else if (sigPtr.rfSignals[s].sourceID > refSigPtr.sig.rfSignals[r].sourceID) {
                RFSignal.rfSignalDiff(currSigDiffs, refSigPtr.sig.rfSignals[r++], null);
            }
            else {
                RTC.avroraPrintHex32(0xBEEF0002);
                RTC.avroraBreak();
            }


            // Add the differences from this iteration
            for (f = 0; f < MoteTrackParams.NBR_FREQCHANNELS; ++f)
                results[f] += currSigDiffs[f];
        }
    }

    /**
     * Implements the <b>UNIDIRECTIONAL</b> signature difference algorithm. <br>
     * @param results[]  place the results for each txPower in this array
     * @param refSigPtr  the reference signature to compare
     * @param sigPtr  the signature to compare
     */
    public static void signatureDiffUnidirectional(short results[], RefSignature refSigPtr, Signature sigPtr) {
        // Not implemented
    }
}
