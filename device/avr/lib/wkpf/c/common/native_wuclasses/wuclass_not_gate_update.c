#include "debug.h"
#include "native_wuclasses.h"

void wuclass_not_gate_setup(wuobject_t *wuobject) {}

void wuclass_not_gate_update(wuobject_t *wuobject) {
  bool input, output;

  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_NOT_GATE_INPUT, &input);

  output = !input;
  wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_NOT_GATE_OUTPUT, output);
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Not gate): !%d => %d\n", input, output);
}
