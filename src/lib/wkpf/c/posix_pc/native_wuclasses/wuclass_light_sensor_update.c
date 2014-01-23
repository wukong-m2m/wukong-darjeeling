#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include "../posix_pc_utils.h"

void wuclass_light_sensor_setup(wuobject_t *wuobject) {}

void wuclass_light_sensor_update(wuobject_t *wuobject) {
	int value = posix_property_get(wuobject, "light_sensor");
	printf("WKPFUPDATE(LightSensor): Sensed light value: %d\n", value);
	wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_LIGHT_SENSOR_CURRENT_VALUE, (int16_t)value);
}
