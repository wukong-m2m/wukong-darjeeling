#include "debug.h"
#include "native_wuclasses.h"

void wuclass_equal_setup(wuobject_t *wuobject) {}

void wuclass_equal_update(wuobject_t *wuobject) {
  int16_t input1, input2;
  bool output;

  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_EQUAL_INPUT1, &input1);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_EQUAL_INPUT1, &input2);

  output = (input1 == input2) ? true : false;
  wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_IF_BOOLEAN_OUTPUT, output);
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(equal): (%d == %d) => %d\n",input1, input2, output);
}
