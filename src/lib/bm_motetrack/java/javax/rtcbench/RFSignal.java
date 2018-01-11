package javax.rtcbench;

import javax.rtc.RTC;

// NOTE: Since NBR_TXPOWERS == 1 in the C code, I've remove the array's for different TX power settings.

public class RFSignal {
    public static final byte MIN_RSSI = 0;
    public static final byte MAX_RSSI = 100;
    public static final byte MAX_RSSI_DIFF = (MAX_RSSI - MIN_RSSI);

    public short sourceID;
    public byte[] rssi = new byte[MoteTrackParams.NBR_FREQCHANNELS];

    public RFSignal()
    {
        byte f=0;

        this.sourceID = 0;
        for (f = 0; f < MoteTrackParams.NBR_FREQCHANNELS; ++f)
            this.rssi[f] = 0;
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
    public static void rfSignalDiff(short results[], RFSignal rfSig1Ptr, RFSignal rfSig2Ptr)
    {
        byte f=0;

        if (rfSig2Ptr == null) {
            for (f = 0; f < MoteTrackParams.NBR_FREQCHANNELS; ++f)
                results[f] = rfSig1Ptr.rssi[f];
        }
        else if ( rfSig2Ptr != null && haveSameID(rfSig1Ptr, rfSig2Ptr) ) {
            for (f = 0; f < MoteTrackParams.NBR_FREQCHANNELS; ++f) {
                // The two rfSignals can be compared.  Return the absolute value
                if (rfSig1Ptr.rssi[f] >= rfSig2Ptr.rssi[f])
                    results[f] = (short) (rfSig1Ptr.rssi[f] - rfSig2Ptr.rssi[f]);
                else
                    results[f] = (short) (rfSig2Ptr.rssi[f] - rfSig1Ptr.rssi[f]);
            }
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
    public static void sortSrcID(RFSignal rfSignals[])
    {
        short size = (short)rfSignals.length;
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
