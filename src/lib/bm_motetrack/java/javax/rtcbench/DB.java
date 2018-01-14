package javax.rtcbench;

import javax.rtc.RTC;
import javax.rtc.Lightweight;

public class DB {
    public final static short REFSIGNATUREDB_SIZE = 257;
    public final static short SIGNATUREDB_SIZE = 74;

    @Lightweight
    public static native void refSignature_get(RefSignature s, short index);
    @Lightweight
    public static native void signature_get(RefSignature s, short index);
}
