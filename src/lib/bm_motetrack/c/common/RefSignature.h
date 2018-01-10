/*
 * Copyright (c) 2005
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/**
 * MoteTrack - Contains most MoteTrack data structures.  In particular it defines
 *             the RefSignature, Signature, and RFSignal.
 * <p>
 * <pre>URL: http://www.eecs.harvard.edu/~konrad/projects/motetrack</pre>
 * @author Konrad Lorincz
 * @version 2.0, January 5, 2005
 */
#ifndef REFSIGNATURE_H
#define REFSIGNATURE_H
#include <stdbool.h>
#include "Point.h"
#include "MoteTrackParams.h"

// ------------------------ RFSignal -----------------------------------
#define MIN_RSSI 0
#define MAX_RSSI 100
#define MAX_RSSI_DIFF (MAX_RSSI - MIN_RSSI)
#define RSSI_CONVERSION_THRESHOLD 350 // For MoteTrack v1.x only - RSSIs below this trashold will not be considered when converting inputRSSI

struct RFSignal
{
    uint16_t sourceID;           // moteID of the source mote
    uint8_t rssi[NBR_FREQCHANNELS][NBR_TXPOWERS];  // received signal strength for each of the freqChan and power levels
};
typedef struct RFSignal RFSignal;
typedef RFSignal* RFSignalPtr;

// ------------------------ Signature -----------------------------------
#define NBR_RFSIGNALS_IN_SIGNATURE 14
uint16_t GlobUniqSignatureID;
struct Signature
{
    uint16_t id;
    RFSignal rfSignals[NBR_RFSIGNALS_IN_SIGNATURE]; // list of rfSignals for a particular location
};
typedef struct Signature Signature;
typedef Signature* SignaturePtr;

// ------------------------ RefSignature -----------------------------------
struct RefSignature
{
    Point location;
    Signature sig;
};
typedef struct RefSignature RefSignature;
typedef RefSignature* RefSignaturePtr;



// ========================== RFSignal ==========================
/**
 *  Initializes an RFSignal struct to a consistent state.
 */
static inline void RFSignal_init(RFSignal *rfSigPtr)
{
    uint8_t f=0, p=0;

    rfSigPtr->sourceID = 0;
    for (f = 0; f < NBR_FREQCHANNELS; ++f)
        for (p = 0; p < NBR_TXPOWERS; ++p)
            rfSigPtr->rssi[f][p] = 0;

}

// /**
//  *  Returns true if the current struct is empty, defined as <code>sourceID=0</code>!
//  */
// static inline bool RFSignal_isEmpty(RFSignal *rfSigPtr)
// {
//     if (rfSigPtr->sourceID == 0)
//         return true;
//     else
//         return false;
// }

/**
 * Returns true if the two parameters have the same sourceID.
 */
static inline bool RFSignal_haveSameID(RFSignal *rfSig1Ptr, RFSignal *rfSig2Ptr)
{
    return rfSig1Ptr->sourceID == rfSig2Ptr->sourceID;
}

// /**
//  * Prints the current RFSignal struct.
//  */
// inline void RFSignal_print(RFSignal *rfSignalPtr)
// {
//     uint8_t f=0, p=0;
//     printfUART("<srcID=%i, rssi={", rfSignalPtr->sourceID);

//     for (f = 0; f < NBR_FREQCHANNELS; ++f) {
//         for (p = 0; p < NBR_TXPOWERS; ++p) {
//             if (f == NBR_FREQCHANNELS-1 && p == NBR_TXPOWERS-1)
//                 { printfUART("%i}>", rfSignalPtr->rssi[f][p]); }
//             else
//                 { printfUART("%i, ", rfSignalPtr->rssi[f][p]); }
//         }
//     }
// }


/*
 * The difference between 2 RFSignals can be computed ONLY if:
 *   1. RFSignal1 is not NULL and RFSignal2 is NULL OR
 *   2. neither RFSignal is NULL, and they have the same sourceID.
 * If one of these two conditions doen't hold, then a FATAL ERROR is generated!
 */
static inline void RFSignal_rfSignalDiff(uint16_t results[NBR_FREQCHANNELS][NBR_TXPOWERS], RFSignal *rfSig1Ptr, RFSignal *rfSig2Ptr)
{
    uint8_t f=0, p=0;

    if (rfSig1Ptr != NULL && rfSig2Ptr == NULL) {
        for (f = 0; f < NBR_FREQCHANNELS; ++f)
            for (p = 0; p < NBR_TXPOWERS; ++p)
                results[f][p] = rfSig1Ptr->rssi[f][p];
    }
    else if ( rfSig1Ptr != NULL && rfSig2Ptr != NULL && RFSignal_haveSameID(rfSig1Ptr, rfSig2Ptr) ) {
        for (f = 0; f < NBR_FREQCHANNELS; ++f) {
            for (p = 0; p < NBR_TXPOWERS; ++p) {
                // The two rfSignals can be compared.  Return the absolute value
                if (rfSig1Ptr->rssi[f][p] >= rfSig2Ptr->rssi[f][p])
                    results[f][p] = rfSig1Ptr->rssi[f][p] - rfSig2Ptr->rssi[f][p];
                else
                    results[f][p] = rfSig2Ptr->rssi[f][p] - rfSig1Ptr->rssi[f][p];
            }
        }
    }
    else {
        avroraPrintHex32(0xBEEF0001);
        asm volatile ("break");
        // printfUART("RefSignature - RFSignal_rfSignalDiff(): FATAL_ERROR! the rfSignals cannot be compared\n", "");
        // printfUART("rfSig1Ptr: (0x%x),  rfSig2Ptr: (0x%x)\n  ->", rfSig1Ptr, rfSig2Ptr);
        // RFSignal_print(rfSig1Ptr);
        // printfUART("\n  ->", "");
        // RFSignal_print(rfSig2Ptr);
        // EXIT_PROGRAM = 1;
        // for (f = 0; f < NBR_FREQCHANNELS; ++f)
        //     for (p = 0; p < NBR_TXPOWERS; ++p)
        //         results[f][p] = MAX_RSSI_DIFF;
    }
}

/**
 * Convert the rssi so that a larger value indicates a stronger signal.
 * @param rssi  the original inverted RSSIvalue
 * @return a value in the range <code>[MIN_RSSI, MAX_RSSI]</code>
 */
static inline uint8_t RFSignal_convertRSSI(uint16_t rssi)
{
#if defined(PLATFORM_MICA2) || defined(PLATFORM_MICA2DOT)
    double adcCounts = (double) rssi;
    double V_batt = 2 * 1.5; // 2AA batteries at 1.5Veach
    double V_rssi = V_batt * adcCounts / 1024.0;
  #ifdef CC1000FREQ_915
    double rssi_dBm = -50.0 * V_rssi - 45.5;  // for CC1000 at 915 MHz!
  #else
    double rssi_dBm = -51.3 * V_rssi - 49.2;  // for CC1000 at 433 and 315 MHz!
  #endif

#else
    // assume CC2420 radio
    // See the CC2420 manual for conversion 
    int16_t rssi_dBm = (int8_t) rssi - 45;
#endif

    // Convert to our own MIN_RSSI to MAX_RSSI range
    uint8_t rssi_Scaled =  MAX_RSSI_DIFF + rssi_dBm;

    if (rssi_Scaled < MIN_RSSI) 
        return MIN_RSSI;
    if (rssi_Scaled > MAX_RSSI)
        return MAX_RSSI;
    else
        return rssi_Scaled;
}


/**
 * Compares two RFSignals by sourceID and returns <br>
 *   -1 if a's sourceID is smaller than b's sourceID <br>
 *    0 if the two sourceIDs' are the same <br>
 *    1 if a's sourceID is greater than b's sourceID <br>
 * Usefull primarily for <code>qsort()</code> from stdlib.h
 */
// static inline int RFSignal_compare(const void *a, const void *b)
// {
//     return ( (*(RFSignal*)a).sourceID - (*(RFSignal*)b).sourceID );
// }


// ---------------- KLDEBUG - TEMP HACK because MSP430 libc doesn't have qsort!!!!! -----------
/**
 * Temporary sort to be used for MSP430, because the MSP430 libc doesn't have qsort!!!
 * @param rfSignals[]  the array of RFSignals to sort by source ID
 * @param size  the size of the rfSignals[] array
 */
static inline void RFSignal_sortSrcID(RFSignal rfSignals[], uint16_t size)
{
    int i = 0;
    int k = 0;
    RFSignal temp;

    for (i = 0; i < size-1; ++i) {
        for (k = i+1; k < size; ++k) {

            if (rfSignals[k].sourceID < rfSignals[i].sourceID) {
                temp = rfSignals[i];
                rfSignals[i] = rfSignals[k];
                rfSignals[k] = temp;
            }
        }
    }
}
// --------------------------------------------------------------------------------------------



// ========================== Signature ==========================
/**
 *  Initialize an Signature struct to a consistent state.
 */
static inline void Signature_init(Signature *sigPtr)
{
    uint8_t i = 0;

    for(i = 0; i < NBR_RFSIGNALS_IN_SIGNATURE; ++i)
        RFSignal_init( &(sigPtr->rfSignals[i]) );
                 
    sigPtr->id = ++GlobUniqSignatureID;
}

// /**
//  * Prints only the signature ID (leaves out the RFSignals).
//  */
// inline void Signature_printHeader(Signature *sigPtr)
// {
//     printfUART("<[Signature]: id=%i>", sigPtr->id);
// }

// /**
//  * Prints the entire Signature.
//  */
// inline void Signature_print(Signature *sigPtr)
// {
//     uint16_t i = 0;
//     printfUART("    ----- Signature (0x%x, sizeof=%i) -----\n", sigPtr, sizeof(*sigPtr));
//     printfUART("      id= %i", sigPtr->id);

//     for (i = 0; i < NBR_RFSIGNALS_IN_SIGNATURE; ++i) {
//         if (i % 3 == 0)
//             {printfUART("\n          ", "");}
//         else
//             {printfUART(" ", "");}

//         RFSignal_print( &(sigPtr->rfSignals[i]) );
//     }
//     printfUART("\n    -----------------------------\n", "");
// }


// ========================== RefSignature ==========================
/**
 * Initialize an RefSignature to a consistent state.
 */
// inline void RefSignature_init(RefSignature *refSigPtr)
// {
//     Point_init( &(refSigPtr->location) );
//     Signature_init( &(refSigPtr->sig) );
// }

// /**
//  * Prints only the signature ID and location.
//  */
// inline void RefSignature_printHeader(RefSignature *refPtr)
// {
//     printfUART("<[RefSignature]: id=%i, (x=%i, y=%i, z=%i)>",
//               refPtr->sig.id, refPtr->location.x, refPtr->location.y, refPtr->location.z);
// }

// /**
//  * Prints the entire RefSignature structure.
//  */
// inline void RefSignature_print(RefSignature *refPtr)
// {
//     printfUART("    ========== RefSignature (0x%x) ==========\n", refPtr);
//     printfUART("      (x=%i, y=%i, z=%i)\n", refPtr->location.x, refPtr->location.y, refPtr->location.z);

//     Signature_print( &(refPtr->sig) );
//     printfUART("    ===========================================\n", "");
// }


/**
 * Implements the <b>BIDIRECTIONAL</b> signature difference algorithm. <br>
 * <b>IMPORTANT:</b> the RFSignals in both RefSignature and Signature MUST be sorted by
 *            sourceID in ascending order!
 * @param results[]  place the results for each txPower in this array
 * @param refSigPtr  the reference signature to compare
 * @param sigPtr  the signature to compare
 */
static inline void RefSignature_signatureDiffBidirectional(uint16_t results[NBR_FREQCHANNELS][NBR_TXPOWERS], 
                                                    RefSignature *refSigPtr, Signature *sigPtr)
{
    uint16_t s = 0, r = 0, f = 0, p = 0;
    uint16_t currSigDiffs[NBR_FREQCHANNELS][NBR_TXPOWERS];

    // (1) - Initialize the results
    for (f = 0; f < NBR_FREQCHANNELS; ++f)
        for (p = 0; p < NBR_TXPOWERS; ++p)
            results[f][p] = 0;

    // (2) - Compute differences, while there are more RFSignals in
    //       either the Signature or the RefSignature
    while( (s < NBR_RFSIGNALS_IN_SIGNATURE && sigPtr->rfSignals[s].sourceID != 0) ||
           (r < NBR_RFSIGNALS_IN_SIGNATURE && refSigPtr->sig.rfSignals[r].sourceID != 0) ) {

        // case 1: there are no more rfSignals in Signature
        if ( !(s < NBR_RFSIGNALS_IN_SIGNATURE && sigPtr->rfSignals[s].sourceID != 0) ) {
            RFSignal_rfSignalDiff(currSigDiffs, &(refSigPtr->sig.rfSignals[r++]), NULL);
        }
        // case 2: there are no more rfSignals in RefSignature
        else if ( !(r < NBR_RFSIGNALS_IN_SIGNATURE && refSigPtr->sig.rfSignals[r].sourceID != 0) ) {
            RFSignal_rfSignalDiff(currSigDiffs, &(sigPtr->rfSignals[s++]), NULL);
        }

        // If we made it this far, then there are more rfSignals in both Signature and RefSignature
        // case 3: there is a match
        else if (sigPtr->rfSignals[s].sourceID == refSigPtr->sig.rfSignals[r].sourceID) {
            RFSignal_rfSignalDiff(currSigDiffs, &(sigPtr->rfSignals[s++]), &(refSigPtr->sig.rfSignals[r++]) );
        }
        // case 4: rfSignal of Signature < rfSignal of RefSignature
        else if (sigPtr->rfSignals[s].sourceID < refSigPtr->sig.rfSignals[r].sourceID) {
            RFSignal_rfSignalDiff(currSigDiffs, &(sigPtr->rfSignals[s++]), NULL);
        }
        // case 5: rfSignal of Signature > rfSignal of RefSignature
        else if (sigPtr->rfSignals[s].sourceID > refSigPtr->sig.rfSignals[r].sourceID) {
            RFSignal_rfSignalDiff(currSigDiffs, &(refSigPtr->sig.rfSignals[r++]), NULL);
        }
        else {
            avroraPrintHex32(0xBEEF0002);
            asm volatile ("break");
            // printfUART("RefSignature - RefSignature_signatureDiffBidirectional): FATAL ERROR! Illegal case\n", "");
            // EXIT_PROGRAM = 1;
        }


        // Add the differences from this iteration
        for (f = 0; f < NBR_FREQCHANNELS; ++f)
            for (p = 0; p < NBR_TXPOWERS; ++p)
                results[f][p] += currSigDiffs[f][p];
    }
}

/**
 * Implements the <b>UNIDIRECTIONAL</b> signature difference algorithm. <br>
 * @param results[]  place the results for each txPower in this array
 * @param refSigPtr  the reference signature to compare
 * @param sigPtr  the signature to compare
 */
static inline void RefSignature_signatureDiffUnidirectional(uint16_t results[NBR_FREQCHANNELS][NBR_TXPOWERS], 
                                                     RefSignature *refSigPtr, Signature *sigPtr)
{
    uint16_t s = 0, r = 0, f = 0, p = 0;
    uint16_t currSigDiffs[NBR_FREQCHANNELS][NBR_TXPOWERS];

    // (1) - Initialize the results
    for (f = 0; f < NBR_FREQCHANNELS; ++f)
        for (p = 0; p < NBR_TXPOWERS; ++p)
            results[f][p] = 0;


    // (2) - Compute differences, while there are more RFSignals in the Signature
    while( s < NBR_RFSIGNALS_IN_SIGNATURE && sigPtr->rfSignals[s].sourceID != 0 ) {

        // case 1: there are no more rfSignals in RefSignature
        if ( !(r < NBR_RFSIGNALS_IN_SIGNATURE && refSigPtr->sig.rfSignals[r].sourceID != 0) ) {
            RFSignal_rfSignalDiff(currSigDiffs, &(sigPtr->rfSignals[s++]), NULL);
        }

        // If we made it this far, then there are more rfSignals in both Signature and RefSignature
        // case 2: there is a match
        else if (sigPtr->rfSignals[s].sourceID == refSigPtr->sig.rfSignals[r].sourceID) {
            RFSignal_rfSignalDiff(currSigDiffs, &(sigPtr->rfSignals[s++]), &(refSigPtr->sig.rfSignals[r++]) );
        }
        // case 3: rfSignal of Signature < rfSignal of RefSignature
        else if (sigPtr->rfSignals[s].sourceID < refSigPtr->sig.rfSignals[r].sourceID) {
            RFSignal_rfSignalDiff(currSigDiffs, &(sigPtr->rfSignals[s++]), NULL);
        }
        // case 4: rfSignal of Signature > rfSignal of RefSignature
        else if (sigPtr->rfSignals[s].sourceID > refSigPtr->sig.rfSignals[r].sourceID) {
            r++;  // do not add the difference in the unidirectional alg.
        }
        else {
            avroraPrintHex32(0xBEEF0003);
            asm volatile ("break");
            // printfUART("RefSignature - RefSignature_signatureDiffUnidirectional): FATAL ERROR! Illegal case\n", "");
            // EXIT_PROGRAM = 1;
        }


        // Add the differences from this iteration
        for (f = 0; f < NBR_FREQCHANNELS; ++f)
            for (p = 0; p < NBR_TXPOWERS; ++p)
                results[f][p] += currSigDiffs[f][p];
    }
}

#endif

