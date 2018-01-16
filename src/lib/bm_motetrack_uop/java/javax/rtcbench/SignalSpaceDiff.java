package javax.rtcbench;

import javax.rtc.RTC;

public class SignalSpaceDiff {
    short refSigIndex;
    short diff;

    public SignalSpaceDiff() {
        init(this);
    }

    public static void init(SignalSpaceDiff ssDiffPtr) {
        ssDiffPtr.refSigIndex = 0;
        ssDiffPtr.diff = 32767; // 65535L; // (2^16-1) because it's 16 bits        
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
    public static void put(SignalSpaceDiff[] SSDiffs, short diff, short refSigIndex)
    {
        short i=0;
        short ssDiffsSize = (short)SSDiffs.length;

        if (diff < SSDiffs[ssDiffsSize-1].diff) {    // we can add it

            // find its place, moving values as we go along
            for (i = (short)(ssDiffsSize-1); i > 0 && diff < SSDiffs[i-1].diff; --i) {
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
    public static void centroidLoc(Point retLocPtr, SignalSpaceDiff[] SSDiffs, short nbrRefSigs)
    {
        short i=0;
        int x=0, y=0, z=0;  // to prevent overflow from adding multiple 16-bit points
        RefSignature currRefSig = new RefSignature();      // RefSignature read from database

        for (i = 0; i < nbrRefSigs; ++i) {
            DB.refSignature_get(currRefSig, SSDiffs[i].refSigIndex);
            x += currRefSig.location.x;
            y += currRefSig.location.y;
            z += currRefSig.location.z;
        }

        // Integer division, may lose precision!
        retLocPtr.x = (short)(x/nbrRefSigs);
        retLocPtr.y = (short)(y/nbrRefSigs);
        retLocPtr.z = (short)(z/nbrRefSigs);
    }
}
