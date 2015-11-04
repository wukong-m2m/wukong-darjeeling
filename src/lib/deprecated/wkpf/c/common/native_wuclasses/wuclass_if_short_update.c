#include "debug.h"
#include "native_wuclasses.h"

void wuclass_if_short_setup(wuobject_t *wuobject) {}

void wuclass_if_short_update(wuobject_t *wuobject) {
  int16_t condition, if_true, if_false, output;

  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_IF_SHORT_CONDITION, &condition);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_IF_SHORT_IF_TRUE, &if_true);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_IF_SHORT_IF_FALSE, &if_false);

  output = condition ? if_true : if_false;
  wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_IF_SHORT_OUTPUT, output);
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(short if): %d ? %d : %d => %d\n", condition, if_true, if_false, output);
}
