package javax.wukong.virtualwuclasses;

import javax.wukong.wkpf.WKPF;
import javax.wukong.wkpf.VirtualWuObject;

public class VirtualMathOpWuObject extends GENERATEDVirtualMathOpWuObject {
    public void update() {
        // TODONR: replace these calls with convenience methods in VirtualWuObject once we get the inheritance issue sorted out.
        short input1 = WKPF.getPropertyShort(this, INPUT1);
        short input2 = WKPF.getPropertyShort(this, INPUT2);
        short operator = WKPF.getPropertyShort(this, OPERATOR);
		short output=0;
		short remainder=0;

      	if (operator==GENERATEDWKPF.ENUM_MATH_OPERATOR_MAX) {
			if (input1>=input2)
				output=input1;
			else
				output=input2;
			remainder=0;
        } else if (operator==GENERATEDWKPF.ENUM_MATH_OPERATOR_MIN) {
			if (input1<=input2)
				output=input1;
			else
				output=input2;
			remainder=0;
        } else if(operator==GENERATEDWKPF.ENUM_MATH_OPERATOR_AVG) {
			output=(short)((input1+input2)/2);
			remainder=0;
        } else if(operator==GENERATEDWKPF.ENUM_MATH_OPERATOR_ADD) {
			output=(short)(input1+input2);
			remainder=0;
        } else if(operator==GENERATEDWKPF.ENUM_MATH_OPERATOR_SUB) {	// input1-input2
			output=(short)(input1-input2);
			remainder=0;
        } else if(operator==GENERATEDWKPF.ENUM_MATH_OPERATOR_MULTIPLY) {
			output=(short)(input1*input2);
			remainder=0;
        } else if(operator==GENERATEDWKPF.ENUM_MATH_OPERATOR_DIVIDE) {	// input1/input2
			if(input2==0) {
				System.out.println("WKPFUPDATE(math):divide by 0 Error");
			} else {
				output=(short)(input1/input2);
				remainder=(short)(input1%input2);
			}
        }

        WKPF.setPropertyShort(this, OUTPUT, output);
        WKPF.setPropertyShort(this, REMAINDER, remainder);
        System.out.println("WKPFUPDATE(math): in1 " + input1 + " in2 " + input2 + " operator " + operator + " output " + output + " remainder " + remainder);
    }
}
