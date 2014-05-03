#include "debug.h"
#include "native_wuclasses.h"

void wuclass_xor_gate_setup(wuobject_t *wuobject) {}

void wuclass_xor_gate_update(wuobject_t *wuobject) {
  bool input1, input2, output;

  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_XOR_GATE_INPUT1, &input1);
  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_XOR_GATE_INPUT2, &input2);

  output = !input1 != !input2;
  wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_XOR_GATE_OUTPUT, output);
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Xor gate): %d xor %d => %d\n", input1, input2, output);
}
