package javax.rtcbench;

import javax.rtc.RTC;

public class RefSignature {
    public Point location;
    public Signature sig;

    public RefSignature() {
        this.location = new Point();
        this.sig = new Signature();
    }

    public static void signatureDiffBidirectional(ShortResults results, RefSignature refSigPtr, Signature sigPtr, ShortResults currSigDiffs)
    {
        short s = 0, r = 0, f = 0; //, p = 0;
        // short[] currSigDiffs = new short[MoteTrackParams.NBR_FREQCHANNELS]; // [NBR_TXPOWERS]; --> Passed from nearestRefSigs estLocAndSend to avoid having to create multiple instances.

        // (1) - Initialize the results
        // for (f = 0; f < MoteTrackParams.NBR_FREQCHANNELS; ++f)
        //     results[f] = 0;
        results.r0 = 0;
        results.r1 = 1;

        // (2) - Compute differences, while there are more RFSignals in
        //       either the Signature or the RefSignature
        RFSignal[] sigPtr_rfSignals = sigPtr.rfSignals;
        RFSignal[] refSigPtr_sig_rfSignals = refSigPtr.sig.rfSignals;
        while( (s < Signature.NBR_RFSIGNALS_IN_SIGNATURE) ||
               (r < Signature.NBR_RFSIGNALS_IN_SIGNATURE) ) {

            RFSignal sigPtr_rfSignals_s = sigPtr_rfSignals[s];
            RFSignal refSigPtr_sig_rfSignals_r = refSigPtr_sig_rfSignals[r];

            if (!(sigPtr_rfSignals_s.sourceID != 0 || refSigPtr_sig_rfSignals_r.sourceID != 0)) {
                break;
            }

            RFSignal rfSignalDiff_par_a;
            RFSignal rfSignalDiff_par_b;

            // case 1: there are no more rfSignals in Signature
            if ( !(s < Signature.NBR_RFSIGNALS_IN_SIGNATURE && sigPtr_rfSignals_s.sourceID != 0) ) {
                // RFSignal.rfSignalDiff(currSigDiffs, refSigPtr_sig_rfSignals_r, null);
                rfSignalDiff_par_a = refSigPtr_sig_rfSignals_r;
                rfSignalDiff_par_b = null;
                r++;
            }
            // case 2: there are no more rfSignals in RefSignature
            else if ( !(r < Signature.NBR_RFSIGNALS_IN_SIGNATURE && refSigPtr_sig_rfSignals_r.sourceID != 0) ) {
                // RFSignal.rfSignalDiff(currSigDiffs, sigPtr_rfSignals_s, null);
                rfSignalDiff_par_a = sigPtr_rfSignals_s;
                rfSignalDiff_par_b = null;
                s++;
            }

            // If we made it this far, then there are more rfSignals in both Signature and RefSignature
            // case 3: there is a match
            else if (sigPtr_rfSignals_s.sourceID == refSigPtr_sig_rfSignals_r.sourceID) {
                // RFSignal.rfSignalDiff(currSigDiffs, sigPtr_rfSignals_s, refSigPtr_sig_rfSignals_r );
                rfSignalDiff_par_a = sigPtr_rfSignals_s;
                rfSignalDiff_par_b = refSigPtr_sig_rfSignals_r;
                r++;
                s++;
            }
            // case 4: rfSignal of Signature < rfSignal of RefSignature
            else if (sigPtr_rfSignals_s.sourceID < refSigPtr_sig_rfSignals_r.sourceID) {
                // RFSignal.rfSignalDiff(currSigDiffs, sigPtr_rfSignals_s, null);
                rfSignalDiff_par_a = sigPtr_rfSignals_s;
                rfSignalDiff_par_b = null;
                s++;
            }
            // case 5: rfSignal of Signature > rfSignal of RefSignature
            else if (sigPtr_rfSignals_s.sourceID > refSigPtr_sig_rfSignals_r.sourceID) {
                // RFSignal.rfSignalDiff(currSigDiffs, refSigPtr_sig_rfSignals_r, null);
                rfSignalDiff_par_a = refSigPtr_sig_rfSignals_r;
                rfSignalDiff_par_b = null;
                r++;
            }
            else {
                rfSignalDiff_par_a = null; // Keep compiler happy
                rfSignalDiff_par_b = null;
                RTC.avroraPrintHex32(0xBEEF0002);
                RTC.avroraBreak();
            }

            RFSignal.rfSignalDiff(currSigDiffs, rfSignalDiff_par_a, rfSignalDiff_par_b);

            // Add the differences from this iteration
            // for (f = 0; f < MoteTrackParams.NBR_FREQCHANNELS; ++f)
            //     results[f] += currSigDiffs[f];
            results.r0 += currSigDiffs.r0;
            results.r1 += currSigDiffs.r1;
        }
    }

    /**
     * Implements the <b>UNIDIRECTIONAL</b> signature difference algorithm. <br>
     * @param results[]  place the results for each txPower in this array
     * @param refSigPtr  the reference signature to compare
     * @param sigPtr  the signature to compare
     */
    public static void signatureDiffUnidirectional(ShortResults results, RefSignature refSigPtr, Signature sigPtr) {
        // Not implemented
    }
}
