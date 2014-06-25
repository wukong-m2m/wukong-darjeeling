package javax.wukong.virtualwuclasses;

import javax.wukong.wkpf.WKPF;
import javax.wukong.wkpf.VirtualWuObject;

public class VirtualThresholdWuObject extends GENERATEDVirtualThresholdWuObject {
    public void update() {
        // TODONR: replace these calls with convenience methods in VirtualWuObject once we get the inheritance issue sorted out.
        short operator = WKPF.getPropertyShort(this, OPERATOR);
        short threshold = WKPF.getPropertyShort(this, THRESHOLD);
        short value = WKPF.getPropertyShort(this, VALUE);
        System.out.println("WKPFUPDATE(Threshold): threshold " + threshold + " value " + value + " operator " + operator );
      	if (((operator == GENERATEDWKPF.ENUM_THRESHOLD_OPERATOR_GT || operator == GENERATEDWKPF.ENUM_THRESHOLD_OPERATOR_GTE) && value > threshold)
      	 || ((operator == GENERATEDWKPF.ENUM_THRESHOLD_OPERATOR_LT || operator == GENERATEDWKPF.ENUM_THRESHOLD_OPERATOR_LTE) && value < threshold)
      	 || ((operator == GENERATEDWKPF.ENUM_THRESHOLD_OPERATOR_GTE || operator == GENERATEDWKPF.ENUM_THRESHOLD_OPERATOR_LTE) && value == threshold)) {
            WKPF.setPropertyBoolean(this, OUTPUT, true);
            System.out.println("WKPFUPDATE(Threshold): threshold " + threshold + " value " + value + " operator " + operator + " -> TRUE");
        } else {
            WKPF.setPropertyBoolean(this, OUTPUT, false);
            System.out.println("WKPFUPDATE(Threshold): threshold " + threshold + " value " + value + " operator " + operator + " -> FALSE");
        }
    }
}
