package javax.wukong.virtualwuclasses;

import javax.wukong.wkpf.WKPF;
import javax.wukong.wkpf.VirtualWuObject;

public class VirtualMultiplexerWuObject extends GENERATEDVirtualMultiplexerWuObject {
    public void update() {
        // TODONR: replace these calls with convenience methods in VirtualWuObject once we get the inheritance issue sorted out.
        short selector = WKPF.getPropertyShort(this, SELECTOR);
        short input1 = WKPF.getPropertyShort(this, INPUT1);
        short input2 = WKPF.getPropertyShort(this, INPUT2);
        short input3 = WKPF.getPropertyShort(this, INPUT3);
        short input4 = WKPF.getPropertyShort(this, INPUT4);
        short current_src = WKPF.getPropertyShort(this, CURRENT_SRC);
        short current_dest = WKPF.getPropertyShort(this, CURRENT_DEST);
        short multiplexer_component_id = WKPF.getPropertyShort(this, ID);
        byte output_port = (byte)(current_dest%100);
        short output_component = (short)(current_dest/100);
        byte[] input_port = new byte[5];
        short[] input_component = new short[5]; //0 for current
        //System.out.println("input1:"+input1+", input2:"+input2+", input3:"+input3+", input4:"+input4);
        input_port[0] = (byte)(current_src%100);
        input_port[1] = (byte)(input1%100);
        input_port[2] = (byte)(input2%100);
        input_port[3] = (byte)(input3%100);
        input_port[4] = (byte)(input4%100);

        input_component[0] = (short)(current_src/100);
        input_component[1] = (short)(input1/100);
        input_component[2] = (short)(input2/100);
        input_component[3] = (short)(input3/100);
        input_component[4] = (short)(input4/100);
        //System.out.println("input1:"+input_component[1]+", input2:"+input_component[2]+", input3:"+input_component[3]+", input4:"+input_component[4]+", output:"+output_component+", currentsrc:"+current_src+", currentdest:"+current_dest);
        boolean rtval = false;
        if (selector< 10 && (input_component[0] !=input_component[2])){
          rtval = WKPF.updateLinking(multiplexer_component_id, input_component[1], input_port[1], output_component, output_port, 
                                           input_component[2], input_port[2], output_component, output_port);
          if (rtval) {
            WKPF.setPropertyShort(this, CURRENT_SRC, input2);
            System.out.println("WKPFUPDATE(Multiplexer): current_src set to input[2]");
          }
        } else if (selector > 10 && (input_component[0] !=input_component[1])){
          rtval = WKPF.updateLinking(multiplexer_component_id, input_component[2], input_port[2], output_component, output_port, 
                                           input_component[1], input_port[1], output_component, output_port);
          if (rtval) {
            WKPF.setPropertyShort(this, CURRENT_SRC, input1);
            System.out.println("WKPFUPDATE(Multiplexer): current_src set to input[1]");
          }
        }
        
        System.out.println("WKPFUPDATE(Multiplexer): triggered"+ selector);
//        if (selector == 0) {
//            WKPF.setPropertyShort(this, OUTPUT, input1);
//            System.out.println("WKPFUPDATE(Multiplexer): selector " + selector + " value " + input1);
//        } else if (selector == 1) {
//            WKPF.setPropertyShort(this, OUTPUT, input2);
//            System.out.println("WKPFUPDATE(Multiplexer): selector " + selector + " value " + input2);
//        } else if (selector == 2) {
//            WKPF.setPropertyShort(this, OUTPUT, input3);
//            System.out.println("WKPFUPDATE(Multiplexer): selector " + selector + " value " + input3);
//        } else {
//            WKPF.setPropertyShort(this, OUTPUT, input4);
//            System.out.println("WKPFUPDATE(Multiplexer): selector " + selector + " value " + input4);
//        }
    }
}
