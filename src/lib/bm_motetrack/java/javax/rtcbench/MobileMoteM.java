package javax.rtcbench;

import javax.rtc.RTC;

public class MobileMoteM {

    public static final byte NBR_HT = 2;
    public static final byte RFSIGNALAVG_HT_SIZE = 20/*NBR_RFSIGNALS_IN_SIGNATURE*/ + 5;
    public static RFSignalAvgHT rfSignalHT;
    public static byte currHT = 0;
    public static short indexNextSigEst = 0;

    

//     /**
//      * Simulates the reception of beacon messages, by populating the RFSignalAvgHT
//      * with beacon messages for read from a file (SignatureDB.h).
//      * This is used for debugging purposes or to run it as a simulation offline.
//      */
//     static inline uint16_t addSignatureFromFile()
//     {
//         // Put stuff in Hashtable
//         uint16_t i = 0;
//         uint8_t f = 0, p = 0, k = 0;
//         RefSignature currRefSig;
//         Signature *sigPtr = &(currRefSig.sig);

//         SignatureDB_get(&currRefSig, indexNextSigEst);
//         indexNextSigEst = (indexNextSigEst+1) % SIGNATUREDB_SIZE;

//         for (k = 0; k < 3; ++k)  // simulates adding multiple samples to the hashtable
//             for (i = 0; i < NBR_RFSIGNALS_IN_SIGNATURE; ++i)
//                 if (sigPtr->rfSignals[i].sourceID != 0)
//                     // add each signal at each freqChan and txPower
//                     for (f = 0; f < NBR_FREQCHANNELS; ++f)
//                         for (p = 0; p < NBR_TXPOWERS; ++p) {
//                             RFSignalAvgHT_put(&rfSignalHT[currHT], sigPtr->rfSignals[i].sourceID,
//                                               f, p, sigPtr->rfSignals[i].rssi[f][p]);
//                         }

//         return sigPtr->id;
//     }

//     /**
//      * Constructs a representative Signature out of the RFSignals stored in the current
//      * RFSignalAvgHT.
//      * @param sigPtr  pointer to the memory location where to store the constructed signature
//      * @param srcAddrBcnMaxRSSIPtr  pointer to the source address from which it received the
//      *     strongest RSSI.
//      */
//     bool constructSignature(Signature *sigPtr, uint16_t *srcAddrBcnMaxRSSIPtr)
//     {
//         uint8_t prevHT = 0;

//         // (1) - Swap RFSignalAvg receive buffers
//         prevHT = currHT;
//         currHT = (currHT+1) % NBR_HT;
//         RFSignalAvgHT_init(&rfSignalHT[currHT], (*rfSignalHTData)[currHT], RFSIGNALAVG_HT_SIZE);


//         // (2) - Construct a Signature
//         RFSignalAvgHT_makeSignature(sigPtr, srcAddrBcnMaxRSSIPtr, &rfSignalHT[prevHT]);
//         if(*srcAddrBcnMaxRSSIPtr == 0) {
//             // printfUART("MobileMote - constructSignatureAndSend(): WARNING! couldn't make signature, hash table is empty\n", "");
//             return false;
//         }
//         else  {
//             return true;
//             // printfUART("constructSignature() - called\n", "");
//         }
//     }



//     /**
//      * Takes the next Signature who's location needs to be estimated from the queue, estimates
//      * its location, and sends a reply.
//      */
//     Point estLocAndSend()
//     {
//         // (1) - Construct a representative signature
//         Point locEst;
//         Signature sig;
//         uint16_t srcAddrBcnMaxRSSI;

//         Point_init(&locEst);
//         Signature_init(&sig);

//         sig.id = addSignatureFromFile();
//         if (constructSignature(&sig, &srcAddrBcnMaxRSSI) == false) {
//             locEst.x = locEst.y = locEst.z = 0;
//             avroraPrintHex32(0xBEEF0006);
//             return locEst;
//         }

//         // (2) - Estimate the Signature's location
//         EstimateLoc_estimateLoc(&locEst, &sig);

//         return locEst;

//         // printfUART("\n********************************************************************************\n", "");
//         // printfUART("MobileMote - estLocAndSend():  ", "");
//         // Signature_printHeader(&sig);
//         // printfUART("  =>  ", "");
//         // Point_print(&locEst);
//         // printfUART("\n********************************************************************************\n\n", "");


//         // // (3) - Send the estimated location
//         // sendReplyLocEstMsgUART(sig.id, &locEst, srcAddrBcnMaxRSSI);
//     }



    public static void motetrack_init_benchmark() {
        rfSignalHT = new RFSignalAvgHT(RFSIGNALAVG_HT_SIZE);
        currHT = 0;
        indexNextSigEst = 0;
    }
}

