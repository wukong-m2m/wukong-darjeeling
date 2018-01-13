package javax.rtcbench;

import javax.rtc.RTC;

public class EstimateLoc {
    // // ----- Parameters -----
    // // (1) - Reference signature selection algorithm  (use one of the two)
    // //#define TH_NEAREST_ALG
    // #define K_NEAREST_ALG

    // #define TH_NEAREST_VAL 128  // percent diff. (i.e. 100*diff2/diff1)
    private static final byte KNEAREST_SIZE = 2;

    // #ifdef TH_NEAREST_ALG
    //     #define MAX_REFSIGS_CONS 9
    // #else
    //     #define MAX_REFSIGS_CONS KNEAREST_SIZE
    // #endif
    private static final byte MAX_REFSIGS_CONS = 2;
    private static final byte BIDIRECTIONAL_ALG  = 0;
    private static final byte UNIDIRECTIONAL_ALG = 1;
    private static final byte signatureDiffAlg = BIDIRECTIONAL_ALG;

    /**
     * Private helper function.  Gets the MAX_REFSIGS_CONS nearest RefSignatures to Signature 
     * in signal space and returns it in the retSSDiffs data structure.
     * @param retSSDiffs[][]  return the indexes to the top RefSignature through this array
     * @param sigPtr  a pointer to the signature to which the RefSignatures should be compared
     */
    // public static void nearestRefSigs(SignalSpaceDiff[][] retSSDiffs, Signature sigPtr)
    public static void nearestRefSigs(Object[] retSSDiffs, Signature sigPtr)
    {
        short i=0, f=0; //, p=0;
        RefSignature currRefSig = new RefSignature();      // RefSignature read from database
        short[] currSigDiffs = new short[MoteTrackParams.NBR_FREQCHANNELS];

        // (1) - Initialize SignalSpaceDiff data structure
        for (f = 0; f < MoteTrackParams.NBR_FREQCHANNELS; ++f)
            for (i = 0; i < MAX_REFSIGS_CONS; ++i)
                SignalSpaceDiff.init(((SignalSpaceDiff[])retSSDiffs[f])[i]);

        // (2) - Get the nearest RefSignatures in signal space and put them in RETssDiffs
        for (i = 0; i < DB.REFSIGNATUREDB_SIZE; ++i) {        // iterate over RefSignature
            // a. get current RefSignature being considered
            DB.refSignature_get(currRefSig, i);

            // b. calculate signal space diffs at all freqChans and txPowers
            if (signatureDiffAlg == BIDIRECTIONAL_ALG)
                RefSignature.signatureDiffBidirectional(currSigDiffs, currRefSig, sigPtr);
            else if (signatureDiffAlg == UNIDIRECTIONAL_ALG)
                RefSignature.signatureDiffUnidirectional(currSigDiffs, currRefSig, sigPtr);
            else {
                // Keep the compiler happy
                for (f = 0; f < MoteTrackParams.NBR_FREQCHANNELS; ++f)
                    currSigDiffs[f] = 0;
                // printfUART("BeaconMote - nearestRefSigs():  FATAL ERROR! neither BIDIRECTIONAL_ALG nor UNIDIRECTIONAL_ALG are defined\n", "");
                RTC.avroraPrintHex32(0xBEEFBEEF);
                RTC.avroraPrintHex32(0x1);
                RTC.avroraBreak();
            }

            // c. try to add curr RefSignatures to top candidates
            for (f = 0; f < MoteTrackParams.NBR_FREQCHANNELS; ++f)
                SignalSpaceDiff.put((SignalSpaceDiff[])retSSDiffs[f], currSigDiffs[f], i);
        }
    }

    /**
     * Estimates the location of this signature.
     * @param retLocPtr  the estimated location should be returned through this pointer
     * @param sigPtr   the signature whos location to estimate
     */
    public static void estimateLoc(Point retLocPtr, Signature sigPtr)
    {
        short f=0; //, p=0; //, r=0;
        // SignalSpaceDiff[][] ssDiffs = new SignalSpaceDiff[MoteTrackParams.NBR_FREQCHANNELS][MAX_REFSIGS_CONS];
        Object[] ssDiffs = new Object[MoteTrackParams.NBR_FREQCHANNELS];
        for (f=0; f<MoteTrackParams.NBR_FREQCHANNELS; f++) {
            SignalSpaceDiff[] ssDiffsarray = new SignalSpaceDiff[MAX_REFSIGS_CONS];
            for (short i=0; i<MAX_REFSIGS_CONS; i++) {
                ssDiffsarray[i] = new SignalSpaceDiff();
            }
            ssDiffs[f] = ssDiffsarray;
        }
        Point[] locEstEachFreqPower = new Point[MoteTrackParams.NBR_FREQCHANNELS];       // centroid for each txPower
        Point[] locCombFreqPower = new Point[MoteTrackParams.NBR_FREQCHANNELS];
        for (f=0; f<MoteTrackParams.NBR_FREQCHANNELS; f++) {
            locEstEachFreqPower[f] = new Point();
            locCombFreqPower[f] = new Point();
        }

        // (1) - Get the nearest RefSignatures to Signature in signal space
        EstimateLoc.nearestRefSigs(ssDiffs, sigPtr);

        // (2) - Figure out how many RefSignatures to include
        //   a) Over each freqChan and txPower
        for (f = 0; f < MoteTrackParams.NBR_FREQCHANNELS; ++f) {
            // #ifdef K_NEAREST_ALG
                SignalSpaceDiff.centroidLoc(locEstEachFreqPower[f], (SignalSpaceDiff[])(ssDiffs[f]), KNEAREST_SIZE);
            // #else  // assume TH_NEAREST_ALG
            //     for (r = 1; r < MAX_REFSIGS_CONS; ++r) {
            //         if ( ((100.0*(double)ssDiffs[f][r].diff)/(double)ssDiffs[f][0].diff) > TH_NEAREST_VAL )
            //             break;
            //     }
            //     SignalSpaceDiff_centroidLocWeighted(&locEstEachFreqPower[f], ssDiffs[f], r);
            // #endif

            locCombFreqPower[f] = locEstEachFreqPower[f];
        }

        //   b) Over all txPowers
        Point.centroidLoc(retLocPtr, locCombFreqPower);
    }
}

