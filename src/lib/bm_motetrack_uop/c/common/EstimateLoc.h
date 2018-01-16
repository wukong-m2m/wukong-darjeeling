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
 * MoteTrack - Location estimation engine.
 * <p>
 * This file contains the engine for estimating locations.  Clients should call 
 * <code>EstimateLoc_estimateLoc()</code> to estimate the location of a signature.
 * The rest of the functions in this file are private/helper functions and should not
 * be called directly by client code.
 *
 * <pre>URL: http://www.eecs.harvard.edu/~konrad/projects/motetrack</pre>
 * @author Konrad Lorincz
 * @version 2.0, January 5, 2005
 */
#ifndef ESTIMATELOC_H
#define ESTIMATELOC_H
#include "MoteTrackParams.h"
#include "RefSignature.h"
#include "RefSignatureDB.h"


// ----- Parameters -----
// (1) - Reference signature selection algorithm  (use one of the two)
//#define TH_NEAREST_ALG
#define K_NEAREST_ALG

#define TH_NEAREST_VAL 128  // percent diff. (i.e. 100*diff2/diff1)
#define KNEAREST_SIZE 2

#ifdef TH_NEAREST_ALG
    #define MAX_REFSIGS_CONS 9
#else
    #define MAX_REFSIGS_CONS KNEAREST_SIZE
#endif


#define BIDIRECTIONAL_ALG  0
#define UNIDIRECTIONAL_ALG 1
uint8_t signatureDiffAlg = BIDIRECTIONAL_ALG;



    // ----------------------- SignalSpaceDiff - private helper functions -------------------------
    struct SignalSpaceDiff {
        uint16_t refSigIndex;
        uint16_t diff;
    };
    typedef struct SignalSpaceDiff SignalSpaceDiff;

    /**
     * Private helper function for data structure <code>SignalSpaceDiff</code>.
     * Initialize struct.
     */
    static inline void SignalSpaceDiff_init(SignalSpaceDiff *ssDiffPtr)
    {
        ssDiffPtr->refSigIndex = 0;
        ssDiffPtr->diff = 65535L; // (2^16-1) because it's 16 bits
    }

    /**
     * Private helper function for data structure <code>SignalSpaceDiff</code>.
     * Tries to add a RefSignature index for a specific freqChan and txPower to an array which stores indices to RefSignatures
     * with smallest differences.  The array is kept sorted by differences, where index 0
     * has the smallest difference.
     * @param SSDiffs[]  the sorted array
     * @param ssDiffsSize  the size of the array
     * @param diff  the new difference to add
     * @param refSigIndex  index to the new RefSignature
     */
    static inline void SignalSpaceDiff_put(SignalSpaceDiff SSDiffs[], uint16_t ssDiffsSize, uint16_t diff, uint16_t refSigIndex)
    {
        uint16_t i=0;

        if (diff < SSDiffs[ssDiffsSize-1].diff) {    // we can add it

            // find its place, moving values as we go along
            for (i = ssDiffsSize-1; i > 0 && diff < SSDiffs[i-1].diff; --i) {
                SSDiffs[i].diff = SSDiffs[i-1].diff;
                SSDiffs[i].refSigIndex = SSDiffs[i-1].refSigIndex;
            }

            // found its place
            SSDiffs[i].diff = diff;
            SSDiffs[i].refSigIndex = refSigIndex;
        }
    }

    /**
     * Private helper function for data structure <code>SignalSpaceDiff</code>.
     * Computes the centroid location of several RefSignatures.
     * @param retLocPtr  return the location through this pointer
     * @param SSDiffs[]  the sorted array
     * @param nbrRefSigs  the number of RefSignatures to computer the centroid over.
     */
    static inline void SignalSpaceDiff_centroidLoc(Point *retLocPtr, SignalSpaceDiff SSDiffs[], uint16_t nbrRefSigs)
    {
        uint16_t i=0;
        uint32_t x=0, y=0, z=0;  // to prevent overflow from adding multiple 16-bit points
        RefSignature currRefSig;      // RefSignature read from database

        for (i = 0; i < nbrRefSigs; ++i) {
            RefSignatureDB_get(&currRefSig, SSDiffs[i].refSigIndex);
            x += currRefSig.location.x;
            y += currRefSig.location.y;
            z += currRefSig.location.z;
        }

        // Integer division, may lose precision!
        retLocPtr->x = x/nbrRefSigs;
        retLocPtr->y = y/nbrRefSigs;
        retLocPtr->z = z/nbrRefSigs;
    }

    // /**
    //  * Private helper function for data structure <code>SignalSpaceDiff</code>.
    //  * Computes the weighted centroid location of several RefSignatures.  The weights are
    //  * inversily proportional with the <code>diff</code> (i.e. smaller diffs are assigned
    //  * a higher weight.
    //  * @param retLocPtr  return the location through this pointer
    //  * @param SSDiffs[]  the sorted array
    //  * @param nbrRefSigs  the number of RefSignatures to computer the centroid over.
    //  */
    // static inline void SignalSpaceDiff_centroidLocWeighted(Point *retLocPtr, SignalSpaceDiff SSDiffs[], uint16_t nbrRefSigs)
    // {
    //     uint16_t i=0;
    //     double x=0, y=0, z=0;  // to prevent overflow from adding multiple 16-bit points
    //     double totalDiffs = 0;
    //     RefSignature currRefSig;      // RefSignature read from database
    //     double currWeight = 0;

    //     // (1) - Determine the totalDiffs to figure out the appropriate weights.
    //     //       Note: We use the diffs as weights, but need to reverse them so that smaller
    //     //       diffs carry a heigher weight.  For this conversion, we need the sum of the weights
    //     for (i = 0; i < nbrRefSigs; ++i)
    //         totalDiffs += SSDiffs[i].diff;

    //     // (2) - Add each one multiplied by its weight
    //     for (i = 0; i < nbrRefSigs; ++i) {
    //         RefSignatureDB_get(&currRefSig, SSDiffs[i].refSigIndex);
    //         currWeight =  2.0/(double)nbrRefSigs - (double)SSDiffs[i].diff/totalDiffs;
    //         x += (double) currRefSig.location.x * currWeight;
    //         y += (double) currRefSig.location.y * currWeight;
    //         z += (double) currRefSig.location.z * currWeight;
    //     }

    //     // Casting to integer, may lose precision!
    //     retLocPtr->x = (uint16_t) x;
    //     retLocPtr->y = (uint16_t) y;
    //     retLocPtr->z = (uint16_t) z;
    // }
    // // --------------------------------------------------------------------------------------------


/**
 * Private helper function.  Gets the MAX_REFSIGS_CONS nearest RefSignatures to Signature 
 * in signal space and returns it in the retSSDiffs data structure.
 * @param retSSDiffs[][]  return the indexes to the top RefSignature through this array
 * @param sigPtr  a pointer to the signature to which the RefSignatures should be compared
 */
static inline void EstimateLoc_nearestRefSigs(SignalSpaceDiff retSSDiffs[NBR_FREQCHANNELS][NBR_TXPOWERS][MAX_REFSIGS_CONS], Signature *sigPtr)
{
    uint16_t i=0, f=0, p=0;
    RefSignature currRefSig;      // RefSignature read from database
    uint16_t currSigDiffs[NBR_FREQCHANNELS][NBR_TXPOWERS];

    // (1) - Initialize SignalSpaceDiff data structure
    for (f = 0; f < NBR_FREQCHANNELS; ++f)
        for (p = 0; p < NBR_TXPOWERS; ++p)
            for (i = 0; i < MAX_REFSIGS_CONS; ++i)
                SignalSpaceDiff_init(&retSSDiffs[f][p][i]);


    // (2) - Get the nearest RefSignatures in signal space and put them in RETssDiffs
    for (i = 0; i < REFSIGNATUREDB_SIZE; ++i) {        // iterate over RefSignature

        // a. get current RefSignature being considered
        RefSignatureDB_get(&currRefSig, i);

        // b. calculate signal space diffs at all freqChans and txPowers
        if (signatureDiffAlg == BIDIRECTIONAL_ALG)
            RefSignature_signatureDiffBidirectional(currSigDiffs, &currRefSig, sigPtr);
        else if (signatureDiffAlg == UNIDIRECTIONAL_ALG)
            RefSignature_signatureDiffUnidirectional(currSigDiffs, &currRefSig, sigPtr);
        else {
            // Keep the compiler happy
            for (f = 0; f < NBR_FREQCHANNELS; ++f)
                for (p = 0; p < NBR_TXPOWERS; ++p)
                    currSigDiffs[f][p] = 0;
            // printfUART("BeaconMote - nearestRefSigs():  FATAL ERROR! neither BIDIRECTIONAL_ALG nor UNIDIRECTIONAL_ALG are defined\n", "");
            avroraPrintHex32(0xBEEFBEEF);
            avroraPrintHex32(0x1);
            asm volatile ("break");
        }

        // c. try to add curr RefSignatures to top candidates
        for (f = 0; f < NBR_FREQCHANNELS; ++f)
            for (p = 0; p < NBR_TXPOWERS; ++p)
                SignalSpaceDiff_put(retSSDiffs[f][p], MAX_REFSIGS_CONS, currSigDiffs[f][p], i);
    }
}

/**
 * Estimates the location of this signature.
 * @param retLocPtr  the estimated location should be returned through this pointer
 * @param sigPtr   the signature whos location to estimate
 */
void EstimateLoc_estimateLoc(Point *retLocPtr, Signature *sigPtr)
{
    uint8_t f=0, p=0; //, r=0;
    SignalSpaceDiff ssDiffs[NBR_FREQCHANNELS][NBR_TXPOWERS][MAX_REFSIGS_CONS];
    Point locEstEachFreqPower[NBR_FREQCHANNELS][NBR_TXPOWERS];       // centroid for each txPower
    Point locCombFreqPower[NBR_FREQCHANNELS*NBR_TXPOWERS];

    // (1) - Get the nearest RefSignatures to Signature in signal space
    EstimateLoc_nearestRefSigs(ssDiffs, sigPtr);

    // (2) - Figure out how many RefSignatures to include
    //   a) Over each freqChan and txPower
    for (f = 0; f < NBR_FREQCHANNELS; ++f)
        for (p = 0; p < NBR_TXPOWERS; ++p) {

            #ifdef K_NEAREST_ALG
                SignalSpaceDiff_centroidLoc(&locEstEachFreqPower[f][p], ssDiffs[f][p], KNEAREST_SIZE);
            #else  // assume TH_NEAREST_ALG
                for (r = 1; r < MAX_REFSIGS_CONS; ++r) {
                    if ( ((100.0*(double)ssDiffs[f][p][r].diff)/(double)ssDiffs[f][p][0].diff) > TH_NEAREST_VAL )
                        break;
                }
                SignalSpaceDiff_centroidLocWeighted(&locEstEachFreqPower[f][p], ssDiffs[f][p], r);
            #endif

            locCombFreqPower[f + p*NBR_FREQCHANNELS] = locEstEachFreqPower[f][p];
        }

    //   b) Over all txPowers
    Point_centroidLoc(retLocPtr, locCombFreqPower, NBR_FREQCHANNELS*NBR_TXPOWERS);
}


#endif
