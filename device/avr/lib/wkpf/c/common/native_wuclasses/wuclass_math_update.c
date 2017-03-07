#include "debug.h"
#include "native_wuclasses.h"

void wuclass_math_op_setup(wuobject_t *wuobject) {}

void wuclass_math_op_update(wuobject_t *wuobject) {
  int16_t input1;
  int16_t input2;
  int16_t op;
  int16_t output = 0;
  int16_t remainder = 0;

  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_MATH_OP_INPUT1, &input1);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_MATH_OP_INPUT2, &input2);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_MATH_OP_OPERATOR, &op);

  if(op==WKPF_ENUM_MATH_OPERATOR_MAX) {
		if (input1>=input2)
			output=input1;
		else if (input2>=input1)
			output=input2;
		remainder=0;
  } else if(op==WKPF_ENUM_MATH_OPERATOR_MIN) {
	  	if (input1<=input2)
			output=input1;
		else if (input2<=input1)
			output=input2;
		remainder=0;
  } else if(op==WKPF_ENUM_MATH_OPERATOR_AVG) {
		output=(input1+input2)/2;
		remainder=0;
  } else if(op==WKPF_ENUM_MATH_OPERATOR_ADD) {
		output=input1+input2;
		remainder=0;
  } else if(op==WKPF_ENUM_MATH_OPERATOR_SUB) {	// input1-input2
		output=input1-input2;
		remainder=0;
  } else if(op==WKPF_ENUM_MATH_OPERATOR_MULTIPLY) {
		output=input1*input2;
		remainder=0;
  } else if(op==WKPF_ENUM_MATH_OPERATOR_DIVIDE) {	// input1/input2
		if(input2==0)
			DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(math): divide by 0 Error ");
	    output=input1/input2;
		remainder=input1%input2;
  }
  wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_MATH_OP_OUTPUT, output);
  wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_MATH_OP_REMAINDER, remainder);
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(math): Native math: input1 %x input2 %x operator %x -> output %x remainder %x\n", input1, input2, op, output, remainder);
}
