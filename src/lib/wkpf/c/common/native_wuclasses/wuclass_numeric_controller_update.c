#include "debug.h"
#include "native_wuclasses.h"

#ifdef ENABLE_WUCLASS_NUMERIC_CONTROLLER

void wuclass_numeric_controller_setup(wuobject_t *wuobject) {}

void wuclass_numeric_controller_update(wuobject_t *wuobject) {
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(NumericController): NOP\n");
}

#endif // ENABLE_WUCLASS_NUMERIC_CONTROLLER
