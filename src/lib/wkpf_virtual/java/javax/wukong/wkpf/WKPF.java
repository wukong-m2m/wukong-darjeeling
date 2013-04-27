package javax.wukong.wkpf;

public class WKPF {
  public static native byte getErrorCode(); // Since we can't raise exceptions, I'll use this to signal whether a call succeeds or not.

  // WuClass and wuobject maintenance
  public static native void registerWuClass(short wuclassId, byte[] properties);
  public static native void createWuObject(short wuclassId, byte portNumber, VirtualWuObject virtualWuObjectInstance); // byte or short? No unsigned byte in Java ><
  public static native void destroyWuObject(byte portNumber);

  // Property access for virtual wuclasses
  public static native short getPropertyShort(VirtualWuObject virtualWuObjectInstance, byte propertyNumber);
  public static native void setPropertyShort(VirtualWuObject virtualWuObjectInstance, byte propertyNumber, short value);
  public static native boolean getPropertyBoolean(VirtualWuObject virtualWuObjectInstance, byte propertyNumber);
  public static native void setPropertyBoolean(VirtualWuObject virtualWuObjectInstance, byte propertyNumber, boolean value);

  // Application startup phase 1: read link table and component map
  public static native void appInitLinkTableAndComponentMap();
  // Application startup phase 2: After this, the Java code should register its virtual wuclasses and create its local instances of virtual wuobjects
  // Application startup phase 3: Create local instances of native wuclasses and process the initvalues file
  public static native void appInitLocalObjectAndInitValues();
  // Application main loop:
  public static native VirtualWuObject select();

  // component-wuobject map related functions
  public static native byte getPortNumberForComponent(short componentId);
  public static native boolean isLocalComponent(short componentId);

  // Who am I?
  public static native short getMyNodeId();

  // Note: need to match definitions in wkpf.h
  public static final byte PROPERTY_TYPE_SHORT                         = 0;
  public static final byte PROPERTY_TYPE_BOOLEAN                       = 1;
  public static final byte PROPERTY_TYPE_REFRESH_RATE                  = 2;
  public static final byte PROPERTY_ACCESS_READONLY           = (byte)(1 << 7);
  public static final byte PROPERTY_ACCESS_WRITEONLY          = (byte)(1 << 6);
  public static final byte PROPERTY_ACCESS_READWRITE = (PROPERTY_ACCESS_READONLY|PROPERTY_ACCESS_WRITEONLY);

  public static final byte OK                                             =  0;
  public static final byte ERR_WUOBJECT_NOT_FOUND                         =  1;
  public static final byte ERR_PROPERTY_NOT_FOUND                         =  2;
  public static final byte ERR_WUCLASS_NOT_FOUND                          =  3;
  public static final byte ERR_READ_ONLY                                  =  4;
  public static final byte ERR_WRITE_ONLY                                 =  5;
  public static final byte ERR_PORT_IN_USE                                =  6;
  public static final byte ERR_WUCLASS_ID_IN_USE                          =  7;
  public static final byte ERR_OUT_OF_MEMORY                              =  8;
  public static final byte ERR_WRONG_DATATYPE                             =  9;
  public static final byte ERR_WUOBJECT_ALREADY_ALLOCATED                 = 10;
  public static final byte ERR_NEED_VIRTUAL_WUCLASS_INSTANCE              = 11;
  public static final byte ERR_NVMCOMM_SEND_ERROR                         = 12;
  public static final byte ERR_NVMCOMM_NO_REPLY                           = 13;
  public static final byte ERR_REMOTE_PROPERTY_FROM_JAVASET_NOT_SUPPORTED = 14;
  public static final byte ERR_SHOULDNT_HAPPEN                    = (byte)0xFF;
}

