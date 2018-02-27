package javax.rtcbench;

import javax.rtc.RTC;

public class Debug {
    public static void printSignature(Signature s) {
        RTC.avroraPrintShort(s.id);
        for (int i=0; i<Signature.NBR_RFSIGNALS_IN_SIGNATURE; i++) {
            RTC.beep(i);
            RTC.avroraPrintShort(s.rfSignals[i].sourceID);
            RTC.avroraPrintShort(s.rfSignals[i].rssi_0);
            RTC.avroraPrintShort(s.rfSignals[i].rssi_1);
        }
        RTC.avroraBreak();
    }

    public static void printRefSignature(RefSignature r) {
        RTC.avroraPrintShort(r.location.x);
        RTC.avroraPrintShort(r.location.y);
        RTC.avroraPrintShort(r.location.z);
        RTC.avroraPrintShort(r.sig.id);
        for (int i=0; i<Signature.NBR_RFSIGNALS_IN_SIGNATURE; i++) {
            RTC.beep(i);
            RTC.avroraPrintShort(r.sig.rfSignals[i].sourceID);
            RTC.avroraPrintShort(r.sig.rfSignals[i].rssi_0);
            RTC.avroraPrintShort(r.sig.rfSignals[i].rssi_1);
        }
        RTC.avroraBreak();
    }

    public static void printRFSignalAvgHT(RFSignalAvgHT h) {
        RTC.avroraPrintShort(h.size);
        RTC.avroraPrintShort(h.capacity);
        for (int i=0; i<h.capacity; i++) {
            RTC.beep(i);
            RTC.avroraPrintShort(h.htData[i].sourceID);
            RTC.avroraPrintShort(h.htData[i].rssiSum_0);
            RTC.avroraPrintShort(h.htData[i].nbrSamples_0);
            RTC.avroraPrintShort(h.htData[i].rssiSum_1);
            RTC.avroraPrintShort(h.htData[i].nbrSamples_1);
        }
        RTC.avroraBreak();
    }
}
