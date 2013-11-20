package javax.wukong.virtualwuclasses;
import javax.wukong.wkpf.WKPF;
import javax.wukong.wkpf.VirtualWuObject;

public class VirtualOrGateWuObject extends GENERATEDVirtualOrGateWuObject {
    public VirtualOrGateWuObject() {
	    // Initialize the wuobject here
	}
	public void update() {
		// Check the update of the properties here
        boolean in1 = WKPF.getPropertyBoolean(this, INPUT1);
        boolean in2 = WKPF.getPropertyBoolean(this, INPUT2);
        if (in1 || in2) {
            WKPF.setPropertyBoolean(this, OUTPUT, true);
            System.out.println("WKPFUPDATE(OrGate):or gate -> TRUE");
        } else {
            WKPF.setPropertyBoolean(this, OUTPUT, false);
            System.out.println("WKPFUPDATE(OrGate):or gate -> FALSE");
        }
	}
}