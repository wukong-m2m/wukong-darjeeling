#include "debug.h"
#include "native_wuclasses.h"

void wuclass_logical_setup(wuobject_t *wuobject) {}

void wuclass_logical_update(wuobject_t *wuobject) {
  bool input1;
  bool input2;
  bool input3;
  bool input4;
  int16_t op;

  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LOGICAL_INPUT1, &input1);
  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LOGICAL_INPUT2, &input2);
  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LOGICAL_INPUT3, &input3);
  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LOGICAL_INPUT4, &input4);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_LOGICAL_OPERATOR, &op);

	if ( ((input1 && input2 && input3 && input4) && op==WKPF_ENUM_LOGICAL_OPERATOR_AND) ||
		 ((input1 || input2 || input3 || input4) && op==WKPF_ENUM_LOGICAL_OPERATOR_OR) ||
		 ((input1 == false) && op==WKPF_ENUM_LOGICAL_OPERATOR_NOT) ||//only not input1
		 ((input1 ^ input2 ^ input3 ^ input4) && op==WKPF_ENUM_LOGICAL_OPERATOR_XOR) ) {
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_LOGICAL_OUTPUT, true);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(logical): Native logical: input1 %x input2 %x input3 %x input4 %x operator %x-> TRUE\n", input1, input2, input3, input4, op);
  }
	else {
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_LOGICAL_OUTPUT, false);
    DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(logical): Native logical: input1 %x input2 %x input3 %x input4 %x operator %x-> FALSE\n", input1, input2, input3, input4, op);
  }
}
