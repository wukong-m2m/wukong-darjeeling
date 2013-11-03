package javax.wukong.virtualwuclasses;
import javax.wukong.wkpf.WKPF;
import javax.wukong.wkpf.VirtualWuObject;

public class VirtualNotGateWuObject extends GENERATEDVirtualNotGateWuObject {
    public VirtualNotGateWuObject() {
	    // Initialize the wuobject here
	}
	public void update() {
		// Check the update of the properties here
        
        boolean input = WKPF.getPropertyBoolean(this, INPUT);
        WKPF.setPropertyBoolean(this, OUTPUT, !input);
        System.out.println("WKPFUPDATE(ANDGate):not gate -> " + !input);
        
	}
}