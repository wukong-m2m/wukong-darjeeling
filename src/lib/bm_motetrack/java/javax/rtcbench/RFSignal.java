package javax.rtcbench;

import javax.rtc.RTC;
import javax.rtc.Lightweight;

// NOTE: Since NBR_TXPOWERS == 1 in the C code, I've remove the array's for different TX power settings.

public class RFSignal {
    public static final byte MIN_RSSI = 0;
    public static final byte MAX_RSSI = 100;
    public static final byte MAX_RSSI_DIFF = (MAX_RSSI - MIN_RSSI);

    public short sourceID;
    public byte rssi_0;
    public byte rssi_1;

    public RFSignal()
    {
    }

    public static void init(RFSignal sigPtr) {
        sigPtr.sourceID = 0;
        sigPtr.rssi_0 = 0;
        sigPtr.rssi_1 = 0;
    }

    public static boolean haveSameID(RFSignal rfSig1Ptr, RFSignal rfSig2Ptr)
    {
        return rfSig1Ptr.sourceID == rfSig2Ptr.sourceID;
    }

    /*
     * The difference between 2 RFSignals can be computed ONLY if:
     *   1. RFSignal1 is not null and RFSignal2 is null OR
     *   2. neither RFSignal is null, and they have the same sourceID.
     * If one of these two conditions doen't hold, then a FATAL ERROR is generated!
     */
    @Lightweight
    public static void rfSignalDiff(short results[], RFSignal rfSig1Ptr, RFSignal rfSig2Ptr)
    {
        byte f=0;

        if (rfSig2Ptr == null) {
            // for (f = 0; f < MoteTrackParams.NBR_FREQCHANNELS; ++f)
                results[0] = rfSig1Ptr.rssi_0;
                results[1] = rfSig1Ptr.rssi_1;
        }
        else if ( rfSig2Ptr != null && haveSameID(rfSig1Ptr, rfSig2Ptr) ) {
            // for (f = 0; f < MoteTrackParams.NBR_FREQCHANNELS; ++f) {
                // The two rfSignals can be compared.  Return the absolute value
                if (rfSig1Ptr.rssi_0 >= rfSig2Ptr.rssi_0)
                    results[0] = (short) (rfSig1Ptr.rssi_0 - rfSig2Ptr.rssi_0);
                else
                    results[0] = (short) (rfSig2Ptr.rssi_0 - rfSig1Ptr.rssi_0);
                if (rfSig1Ptr.rssi_1 >= rfSig2Ptr.rssi_1)
                    results[1] = (short) (rfSig1Ptr.rssi_1 - rfSig2Ptr.rssi_1);
                else
                    results[1] = (short) (rfSig2Ptr.rssi_1 - rfSig1Ptr.rssi_1);
            // }
        }
        else {
            RTC.avroraPrintHex32(0xBEEF0001);
            RTC.avroraBreak();
        }
    }

    /**
     * Convert the rssi so that a larger value indicates a stronger signal.
     * @param rssi  the original inverted RSSIvalue
     * @return a value in the range <code>[MIN_RSSI, MAX_RSSI]</code>
     */
    public static byte convertRSSI(short rssi)
    {
        // assume CC2420 radio
        // See the CC2420 manual for conversion 
        short rssi_dBm = (short) (rssi - 45);

        // Convert to our own MIN_RSSI to MAX_RSSI range
        short rssi_Scaled = (short)(MAX_RSSI_DIFF + rssi_dBm);

        if (rssi_Scaled < MIN_RSSI) 
            return MIN_RSSI;
        if (rssi_Scaled > MAX_RSSI)
            return MAX_RSSI;
        else
            return (byte)rssi_Scaled;
    }

    // ---------------- KLDEBUG - TEMP HACK because MSP430 libc doesn't have qsort!!!!! -----------
    /**
     * Temporary sort to be used for MSP430, because the MSP430 libc doesn't have qsort!!!
     * @param rfSignals[]  the array of RFSignals to sort by source ID
     * @param size  the size of the rfSignals[] array
     */
    public static void sortSrcID(RFSignal rfSignals[], short size)
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
}
