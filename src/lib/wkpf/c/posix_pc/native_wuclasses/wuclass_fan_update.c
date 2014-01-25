#include <stdio.h>
#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include "../posix_pc_utils.h"

void wuclass_fan_setup(wuobject_t *wuobject) {}

void wuclass_fan_update(wuobject_t *wuobject) {
  bool onOff;
  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_FAN_ON_OFF, &onOff);
  posix_property_put(wuobject, "fan", onOff);
  printf("WKPFUPDATE(Fan): Setting fan to: %x\n", onOff);
}
