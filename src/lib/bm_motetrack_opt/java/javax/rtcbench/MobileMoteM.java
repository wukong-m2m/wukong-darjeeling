package javax.rtcbench;

import javax.rtc.RTC;

public class MobileMoteM {

    public static final byte NBR_HT = 2;
    public static final byte RFSIGNALAVG_HT_SIZE = 20/*NBR_RFSIGNALS_IN_SIGNATURE*/ + 5;
    public static RFSignalAvgHT[] rfSignalHT;
    public static byte currHT = 0;
    public static short indexNextSigEst = 0;

    /**
     * Simulates the reception of beacon messages, by populating the RFSignalAvgHT
     * with beacon messages for read from a file (SignatureDB.h).
     * This is used for debugging purposes or to run it as a simulation offline.
     */
    public static short addSignatureFromFile(RefSignature currRefSig)
    {
        // Put stuff in Hashtable
        short i = 0;
        byte f = 0, p = 0, k = 0;
        // RefSignature currRefSig  = new RefSignature(); --> Passed all the way from estLocAndSend to avoid having to create multiple instances.
        Signature sigPtr = currRefSig.sig;

        DB.signature_get(currRefSig, indexNextSigEst);
        indexNextSigEst = (short)((indexNextSigEst+1) % DB.SIGNATUREDB_SIZE);

        for (k = 0; k < 3; ++k)  // simulates adding multiple samples to the hashtable
            for (i = 0; i < Signature.NBR_RFSIGNALS_IN_SIGNATURE; ++i) {
                RFSignal sigPtr_rfSignals_i = sigPtr.rfSignals[i];
                if (sigPtr_rfSignals_i.sourceID != 0) {
                    // add each signal at each freqChan and txPower
                    // for (f = 0; f < MoteTrackParams.NBR_FREQCHANNELS; ++f) {
                        RFSignalAvgHT.put(rfSignalHT[currHT], sigPtr_rfSignals_i.sourceID, (byte)0, sigPtr_rfSignals_i.rssi_0);
                        RFSignalAvgHT.put(rfSignalHT[currHT], sigPtr_rfSignals_i.sourceID, (byte)1, sigPtr_rfSignals_i.rssi_1);
                    // }
                }
            }
        return sigPtr.id;
    }

    /**
     * Constructs a representative Signature out of the RFSignals stored in the current
     * RFSignalAvgHT.
     * @param sigPtr  pointer to the memory location where to store the constructed signature
     * @param srcAddrBcnMaxRSSIPtr  pointer to the source address from which it received the
     *     strongest RSSI.
     */
    public static short constructSignature(Signature sigPtr)
    {
        short srcAddrBcnMaxRSSIPtr;
        byte prevHT = 0;

        // (1) - Swap RFSignalAvg receive buffers
        prevHT = currHT;
        currHT = (byte)((currHT+1) % NBR_HT);
        RFSignalAvgHT.init(rfSignalHT[currHT]);


        // (2) - Construct a Signature
        srcAddrBcnMaxRSSIPtr = RFSignalAvgHT.makeSignature(sigPtr, rfSignalHT[prevHT]);

        return srcAddrBcnMaxRSSIPtr;

        // if(*srcAddrBcnMaxRSSIPtr == 0) {
        //     // printfUART("MobileMote - constructSignatureAndSend(): WARNING! couldn't make signature, hash table is empty\n", "");
        //     return false;
        // }
        // else  {
        //     return true;
        //     // printfUART("constructSignature() - called\n", "");
        // }
    }


    /**
     * Takes the next Signature who's location needs to be estimated from the queue, estimates
     * its location, and sends a reply.
     */
    public static Point estLocAndSend()
    {
        // (1) - Construct a representative signature
        Point locEst = new Point();
        Signature sig = new Signature();
        RefSignature refSig = new RefSignature();
        short srcAddrBcnMaxRSSI;

        Point.init(locEst);
        Signature.init(sig);

        sig.id = addSignatureFromFile(refSig);
        if ((srcAddrBcnMaxRSSI = constructSignature(sig)) == 0) {
            locEst.x = locEst.y = locEst.z = 0;
            RTC.avroraPrintHex32(0xBEEF0006);
            return locEst;
        }

        // (2) - Estimate the Signature's location
        EstimateLoc.estimateLoc(locEst, sig, refSig);

        return locEst;
    }



    public static void motetrack_init_benchmark() {
        rfSignalHT = new RFSignalAvgHT[NBR_HT];
        for (int i=0; i<NBR_HT; i++) {
            rfSignalHT[i] =  new RFSignalAvgHT(RFSIGNALAVG_HT_SIZE);
        }
        currHT = 0;
        indexNextSigEst = 0;
    }
}

