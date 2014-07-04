#include "debug.h"
#include "native_wuclasses.h"

void wuclass_if_boolean_setup(wuobject_t *wuobject) {}

void wuclass_if_boolean_update(wuobject_t *wuobject) {
  bool condition, if_true, if_false, output;

  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_IF_BOOLEAN_CONDITION, &condition);
  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_IF_BOOLEAN_IF_TRUE, &if_true);
  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_IF_BOOLEAN_IF_FALSE, &if_false);

  output = condition ? if_true : if_false;
  wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_IF_BOOLEAN_OUTPUT, output);
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(boolean if): %d ? %d : %d => %d\n", condition, if_true, if_false, output);
}
