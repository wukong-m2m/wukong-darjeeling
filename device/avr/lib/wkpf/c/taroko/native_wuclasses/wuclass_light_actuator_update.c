#include "debug.h"
#include "native_wuclasses.h"

void wuclass_light_actuator_setup(wuobject_t *wuobject) {}

void wuclass_light_actuator_update(wuobject_t *wuobject) {
  bool onOff;
  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LIGHT_ACTUATOR_ON_OFF, &onOff);


  DARJEELING_PRINTF("WKPFUPDATE(Light): Setting light to: %x\n", onOff);
}
