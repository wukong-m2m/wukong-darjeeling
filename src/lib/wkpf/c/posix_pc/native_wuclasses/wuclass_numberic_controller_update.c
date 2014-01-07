#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"

void wuclass_numeric_controller_setup(wuobject_t *wuobject) {}

void wuclass_numeric_controller_update(wuobject_t *wuobject) {
  DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(NumericController): NOP\n");
}
