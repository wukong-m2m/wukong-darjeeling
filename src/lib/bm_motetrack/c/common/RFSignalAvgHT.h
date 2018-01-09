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
 * MoteTrack - Contains the RFSignalAvgHT and RFSignalAvg data structures.
 * <p>
 * RFSignalAvgHT is used by the mobile mote to construct a Signature from
 * multiple beacon messages.  When constructing the a Signature, the average
 * RSSI value is used for each <code><sourceID, freqChan, txPower></code> tuple.
 * The hashtable is keyed by moteID, which allows for constant addition
 * (addition has to be fast because the mobile mote receives beacon messages
 * with high frequency).
 * <p>                                       
 * <pre>URL: http://www.eecs.harvard.edu/~konrad/projects/motetrack</pre>
 * @author Konrad Lorincz
 * @version 2.0, January 5, 2005
 */   
#ifndef RFSIGNALAVGHT_H
#define RFSIGNALAVGHT_H
//#include <stdlib.h>
#include "MTMsgs.h"
#include "RefSignature.h"
#include "MoteTrackParams.h"
#include "PrintfUART.h"



// ------------------------ RFSignalAvg -----------------------------------
struct RFSignalAvg 
{  
    uint16_t sourceID;                 // moteID of the source mote
    uint16_t rssiSum[NBR_FREQCHANNELS][NBR_TXPOWERS];    // the sum of the RSSIs added and to be averaged
    uint8_t nbrSamples[NBR_FREQCHANNELS][NBR_TXPOWERS]; // the number of RSSIs added    
};
typedef struct RFSignalAvg RFSignalAvg;
typedef RFSignalAvg* RFSignalAvgPtr;


// ------------------------ RFSignalAvgHT -----------------------------------
struct RFSignalAvgHT 
{   
    RFSignalAvg *htData;  // points to the data array
    uint16_t size;
    uint16_t capacity;
};
typedef struct RFSignalAvgHT RFSignalAvgHT;
typedef RFSignalAvgHT* RFSignalAvgHTPtr;



// =========================== Interface ==============================

/*inline void RFSignalAvgHT_init(RFSignalAvgHT *HPtr, RFSignalAvg htData[], uint16_t hashCapacity);
inline result_t RFSignalAvgHT_put(RFSignalAvgHT *HPtr, uint16_t newSourceID, uint16_t newRssi, uint8_t txPowerIntex);
inline result_t RFSignalAvgHT_makeSignature(RFSignalAvgHT *HPtr, SignaturePtr sigPtr);
inline void RFSignalAvgHT_print(RFSignalAvgHT *HPtr);
*/



// ========================== RFSignalAvg ==========================
/**
 * Initialize an RFSignalAvg struct to a consistent state.  It's considered empty if
 * sourceID = 0. 
 */
void RFSignalAvg_init(RFSignalAvg *rfSigPtr)
{
    uint8_t f = 0, p = 0;
    rfSigPtr->sourceID = 0;
    for (f = 0; f < NBR_FREQCHANNELS; ++f) {
        for (p = 0; p < NBR_TXPOWERS; ++p) {
            rfSigPtr->rssiSum[f][p] = 0;
            rfSigPtr->nbrSamples[f][p] = 0;
        }
    }
}

/**
 * Reverse Compares two RFSignalAvg by rssiSum[0], (at TXPower index 0) and returns <br>
 * Sorting with this function will end up with an array that has the largest values at the beginning!
 * Usefull primarily for <code>qsort()</code> from stdlib.h
 * @return
 *   -1 if a's rssiSum[0] is greater than b's rssiSum[0] <br>
 *    0 if the two rssiSum[0] 's are the same <br>
 *    1 if a's rssiSum[0] is smaller than b's rssiSum[0] <br>
 */
inline int RFSignalAvg_revCompareRSSI(const void *a, const void *b)
{
    return ( (*(RFSignalAvg*)b).rssiSum[0][0] - (*(RFSignalAvg*)a).rssiSum[0][0] );
}

/**
 * Prints the current RFSignalAvg.
 */
inline void RFSignalAvg_print(RFSignalAvg *sigPtr)
{
    uint8_t f = 0, p = 0;
    printfUART("<srcID=%i, rssiSum ={", sigPtr->sourceID);
    for (f = 0; f < NBR_FREQCHANNELS; ++f) {
        for (p = 0; p < NBR_TXPOWERS; ++p) {
            if (f < NBR_FREQCHANNELS-1 && p < NBR_TXPOWERS-1)
                {printfUART("%i/%i, ", sigPtr->rssiSum[f][p], sigPtr->nbrSamples[f][p]);}
            else
                {printfUART("%i/%i}>", sigPtr->rssiSum[f][p], sigPtr->nbrSamples[f][p]);}            
        }
    }
}


// ========================== RFSignalAvgHT ==========================
/**
 * Initialize an RFSignalAvg struct to a consistent state.
 * @param HPtr  pointer to a RFSignalAvgHT struct
 * @param htData  the data or memory
 * @hashCapacity  the number of elements that can be stored.  It should be the size
 * of the htData array.
 */
inline void RFSignalAvgHT_init(RFSignalAvgHT *HPtr, RFSignalAvg htData[], uint16_t hashCapacity)
{   
    uint16_t i = 0;
    HPtr->htData = htData;
    HPtr->capacity = hashCapacity;
    HPtr->size = 0;

    for (i = 0; i < HPtr->capacity; ++i)
        RFSignalAvg_init( &(HPtr->htData[i]) );        
}               

/**
 * Prints the current RFSignalAvgHT.
 */
inline void RFSignalAvgHT_print(RFSignalAvgHT *HPtr)
{
    uint8_t i = 0;
    printfUART("    ----- RFSignalAvgHT (0x%x, sizeof=%i) -----\n", HPtr, sizeof(*HPtr));
    printfUART("      <capacity=%i (0x%x), size=%i (0x%x)>", 
        HPtr->capacity, &(HPtr->capacity), HPtr->size, &(HPtr->size));

    for (i = 0; i < HPtr->capacity; ++i) {
        if (i % 3 == 0)  {printfUART("\n          ", "");}
        else             {printfUART(" ", "");}

        RFSignalAvg_print(&(HPtr->htData[i]));        
    }
    printfUART("\n    -----------------------------------\n", "");
}



/**
 * Tries to add the content of a new beacon message to this RFSignalAvgHT.  If no cell entry exists
 * for this sourceID, then it creates one.  Otherwise, it just adds the newRSSI to the cell with
 * newSourceID in the f frequency channel index and p power index.  If the hashtable is full, 
 * then it drops the new beacon message.
 * <b>IMPORTANT:</b> If <code>sourceID == 0</code>, then the cell is considered empty 
 * (i.e. no beaconMote may have an ID of 0)!
 * @param HPtr  the current RFSignalAvgHT struct
 * @param newSrcID  the sourceID corresponding to <code>newRssi</code>
 * @param f  the frequency channel index for this beacon message
 * @param p  the transmission power index for this beacon message
 * @param newRssi  the RSSI value
 * @return <code>SUCCESS</code>, if it was added; <code>FAIL</code> if it couldn't be added
 */
inline result_t RFSignalAvgHT_put(RFSignalAvgHT *HPtr, uint16_t newSrcID, uint8_t f, uint8_t p, uint16_t newRssi)
{
    // very simple hash function, but works well because most sourceID's are below HPtr->capacity
    uint16_t hashID = (newSrcID*13) % HPtr->capacity;
    uint16_t i = hashID;

    do {
        // add rfSignal if the current cell has no entry (i.e. nbrSamples == 0)
        // or the current cell entry is for the same sourceID
        if (HPtr->htData[i].sourceID == 0) {
            HPtr->htData[i].sourceID = newSrcID;  // this only matters when nbrSamples[i] == 0!
            HPtr->size++;
        }
        if (HPtr->htData[i].sourceID == newSrcID) {
            HPtr->htData[i].rssiSum[f][p] += newRssi;   // DANGER! if rssiSum is uint8_t there may be an overflow problem!
            HPtr->htData[i].nbrSamples[f][p]++;
            return SUCCESS;                                                 
        }
        i = (i + 1) % HPtr->capacity;
    } while (i != hashID);

    // If we didn't find a cell to place our entry, then is means the hastable 
    // is full (i.e. capacity == size), otherwise somethings has gone wrong
    if (HPtr->size == HPtr->capacity) {
        printfUART("RFSignalAvgHT - put(): WARNING! hash table is full!  Can't add new entry\n", "");
        return FAIL;
    }
    else {
        printfUART("RFSignalAvgHT - put(): FATAL ERROR! hash table is inconsistent!  Dumping hash table and exiting program ...\n", "");
        printfUART("              - trying to add: <srcID=%i, RSSI=%i, txPowerIndex=%i>\n", newSrcID, newRssi, p);
        RFSignalAvgHT_print(HPtr);        
        EXIT_PROGRAM = 1;
        return FAIL;
    }
}


/** 
 * Constructs a Signature from the Hashtable HPtr.  The Signature will contain the 
 * RFSignals with the strongest RSSIs (from HPtr) and sorted by sourceID.  All decisions
 * regarding the strongest RSSIs is based on freqChan index 0 and txPower index 0! <br>
 * <b>WARNING:</b>  After calling this  function, HPtr  becomes inconsistent because some 
 * of its data gets overwritten; therefore it should no longer be used.
 *
 * @param retSigPtr  return the constructed signature through this pointer
 * @param retSrcIDMaxRSSIPtr  return the sourceID of RFSignal with max RSSI through this pointer
 * @param HPtr  the RFSignalAvgHT structure.
 */
inline void RFSignalAvgHT_makeSignature(Signature *retSigPtr, uint16_t *retSrcIDMaxRSSIPtr, RFSignalAvgHT *HPtr)
{
    uint16_t i = 0, k = 0, f = 0, p = 0;
    uint16_t maxRssiIndex = 0;

    // (1) - If the hashtable is empty, then we can't construct a Signature
    if (HPtr->size == 0) {
        *retSrcIDMaxRSSIPtr = 0;
        return;      // the hash table is empty, so can't construct Signature    
    }    

       
    // (2) - If we can include all RFSignals, there is no need to first sort the hashtable by strongest RSSI
    if (HPtr->size <= NBR_RFSIGNALS_IN_SIGNATURE) {
        // (a) Initialize the signature
        //Signature_init(retSigPtr);
        k = -1;
        maxRssiIndex = 0;

        // (b) Construct the Signature
        for (i = 0; i < HPtr->capacity; ++i) {
            if (HPtr->htData[i].sourceID > 0) {  // ASSUMING sourceID=0 means the cell is empty!
                // we have a cell entry, add it to the Signature
            
                retSigPtr->rfSignals[++k].sourceID = HPtr->htData[i].sourceID;

                for (f = 0; f < NBR_FREQCHANNELS; ++f) {
                    for (p = 0; p < NBR_TXPOWERS; ++p) {
                        if (HPtr->htData[i].nbrSamples[f][p] > 0) {
                            // WARNING! integer division, may lose some precision!
                            retSigPtr->rfSignals[k].rssi[f][p] = HPtr->htData[i].rssiSum[f][p] / HPtr->htData[i].nbrSamples[f][p];
                        }
                        else
                            retSigPtr->rfSignals[k].rssi[f][p] = 0;
                    }
                }
                
                // base decision on the 1st freqChan and strongest TXPower (assuming it is index 0)!!!
                if (retSigPtr->rfSignals[k].rssi[0][0] > retSigPtr->rfSignals[maxRssiIndex].rssi[0][0])
                    maxRssiIndex = k;                       
            }    
        }
        
        *retSrcIDMaxRSSIPtr = retSigPtr->rfSignals[maxRssiIndex].sourceID;

        // (c) Sort the RFSignals in Signature by sourceID

        //printfUART("\nBefore sort:\n", "");
        //for (i = 0; i < HPtr->size; ++i)
        //    printfUART("%i  ", retSigPtr->rfSignals[i].sourceID);

        // KLDEBUG - should use qsort.  However MSP430 libc doesn't define qsort!!!!
        // #if defined(PLATFORM_TELOS)
            RFSignal_sortSrcID(retSigPtr->rfSignals, HPtr->size);
        // #else
        //    qsort(retSigPtr->rfSignals, HPtr->size, sizeof(RFSignal), RFSignal_compare);
        // #endif

        //printfUART("\nAfter sort:\n", "");
        //for (i = 0; i < HPtr->size; ++i)
        //    printfUART("%i  ", retSigPtr->rfSignals[i].sourceID);

    }

    // (3) - We can't include all RFSignals, so sort them by strongest RSSIs 
    //       and include only the strongest ones
    else {
        // (a) Initialize the signature
        Signature_init(retSigPtr);


        // (b) IMPORTANT Compute the rssiAvg !!!  We store the results in HPtr, starting
        //     from index 0 and going to index HPtr->size.  Note, we are destroying the HPtr!
        k = 0;
        for (i = 0; i < HPtr->capacity; ++i) {
            if (HPtr->htData[i].sourceID > 0) {  // ASSUMING sourceID=0 means the cell is empty!    
            
                HPtr->htData[k].sourceID = HPtr->htData[i].sourceID;

                for (f = 0; f < NBR_FREQCHANNELS; ++f) {
                    for (p = 0; p < NBR_TXPOWERS; ++p) {
                        if (HPtr->htData[i].nbrSamples[f][p] > 0) {
                            // WARNING 1: placeing result in the k-th cell of HPtr, overwriting what was there!
                            // WARNING 2: integer division, may lose some precision!
                            HPtr->htData[k].rssiSum[f][p] = HPtr->htData[i].rssiSum[f][p] / HPtr->htData[i].nbrSamples[f][p]; 
                            HPtr->htData[k].nbrSamples[f][p] = 1;  // statement not necessary, but keeps datastructure consistent
                        }
                        else
                            HPtr->htData[k].rssiSum[f][p] = 0;
                    }
                }
                k++;
            }               
        }


        // (c) Sort by RSSI so that large RSSIs are at the beginning of the array
        // KLDEBUG -- temporarily commented the line below because MSP430 doesn't define qsort, and 
        // otherwise it does not compile.  SHOULD NOT BE COMMENTED!!!!!!!!!!!!!!!!!!!!!!!
        // qsort(HPtr->htData, HPtr->size, sizeof(RFSignalAvg), RFSignalAvg_revCompareRSSI);

        // at this point the strongest RSSI is at 1st freqChan, index 0
        *retSrcIDMaxRSSIPtr = HPtr->htData[0].sourceID;
        
                                                          
        // (d) IMPORTANT: construct the Signature with the strongest RSSIs  
        //     based on the 1st freqChan and strongest TXPower (assuming it is index 0)!!!
        // we need to copy the RFSignals in reverse order because 
        // the larger RSSIs are at the end of the array
        for (i = 0; i < NBR_RFSIGNALS_IN_SIGNATURE && i < HPtr->size; ++i) {
         
            retSigPtr->rfSignals[i].sourceID = HPtr->htData[i].sourceID;
            for (f = 0; f < NBR_FREQCHANNELS; ++f)
                for (p = 0; p < NBR_TXPOWERS; ++p)
                    retSigPtr->rfSignals[i].rssi[f][p] = HPtr->htData[i].rssiSum[f][p];
        }


        // (e) Sort the RFSignals in Signature by sourceID
        // KLDEBUG - should use qsort.  However MSP430 libc doesn't define qsort!!!!
        // #if defined(PLATFORM_TELOS)
            RFSignal_sortSrcID(retSigPtr->rfSignals, HPtr->size);
        // #else  // PLATFORM_MICAZ || PLATFORM_MICA2 || PLATFORM_MICA2DOT
        //    qsort(retSigPtr->rfSignals, HPtr->size, sizeof(RFSignal), RFSignal_compare);
        // #endif
    }
}

#endif

