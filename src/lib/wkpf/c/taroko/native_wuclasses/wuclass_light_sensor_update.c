#include "debug.h"
#include "native_wuclasses.h"

void wuclass_light_sensor_setup(wuobject_t *wuobject) {}

void wuclass_light_sensor_update(wuobject_t *wuobject) {
  // Just return a value
  wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_LIGHT_SENSOR_CURRENT_VALUE, 42);
}
