package java.lang;

import javax.rtc.RTC;

public class Exception extends Throwable
{
    // Must match numbers in execution.h

    public final static short ARITHMETIC_EXCEPTION = 1;
    public final static short ARRAYINDEXOUTOFBOUNDS_EXCEPTION = 2;
    public final static short ARRAYSTORE_EXCEPTION = 3;
    public final static short CLASSCAST_EXCEPTION = 4;
    public final static short CLASSUNLOADED_EXCEPTION = 5;
    public final static short ILLEGALARGUMENT_EXCEPTION = 6;
    public final static short ILLEGALTHREADSTATE_EXCEPTION = 7;
    public final static short INDEXOUTOFBOUNDS_EXCEPTION = 8;
    public final static short INFUSIONUNLOADDEPENDENCY_EXCEPTION = 9;
    public final static short NATIVEMETHODNOTIMPLEMENTED_ERROR = 10;
    public final static short NULLPOINTER_EXCEPTION = 11;
    public final static short OUTOFMEMORY_ERROR = 12;
    public final static short RUNTIME_EXCEPTION = 13;
    public final static short STACKOVERFLOW_ERROR = 14;
    public final static short STRINGINDEXOUTOFBOUNDS_EXCEPTION = 15;
    public final static short VIRTUALMACHINE_ERROR = 16;

    short type;
    String message;
    Exception cause;

    public Exception()
    {
    };

    public Exception(short type)
    {
        // this.type = type;
        RTC.terminateOnException(type);
    }

    public Exception(short type, String message)
    {
        // this.type = type;
        // this.message = message;
        RTC.terminateOnException(type);
    }

    public Exception(short type, Exception cause)
    {
        // this.type = type;
        // this.cause = cause;
        RTC.terminateOnException(type);
    }

    public Exception(short type, String message, Exception cause)
    {
        // this.type = type;
        // this.message = message;
        // this.cause = cause;
        RTC.terminateOnException(type);
    }

    public String getMessage()
    {
        return message;
    }

    public short getType()
    {
        return type;
    }
}