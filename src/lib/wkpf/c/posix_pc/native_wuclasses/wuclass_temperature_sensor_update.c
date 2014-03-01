#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include "../posix_pc_utils.h"

void wuclass_temperature_sensor_setup(wuobject_t *wuobject) {
	// Just get a value to make sure the file is created even if the object's not used in the FBP
	posix_property_get(wuobject, "temperature_sensor");
}

void wuclass_temperature_sensor_update(wuobject_t *wuobject) {
	int value = posix_property_get(wuobject, "temperature_sensor");
	DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Temperature sensor): Sensed light value: %d\n", value);
	wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_TEMPERATURE_SENSOR_CURRENT_TEMPERATURE, (int16_t)value);
}
