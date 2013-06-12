#include "debug.h"
#include "native_wuclasses.h"

void wuclass_generic_setup(wuobject_t *wuobject) {}

void wuclass_generic_update(wuobject_t *wuobject) {
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Generic): Update called for generic wuclass\n");
  wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_GENERIC_DUMMY, 42);
}
