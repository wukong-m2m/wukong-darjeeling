#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"

void wuclass_light_sensor_setup(wuobject_t *wuobject) {}

void wuclass_light_sensor_update(wuobject_t *wuobject) {
	// Rotate through values 0 to 255 in steps of 10
	int16_t value;
	wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_LIGHT_ACTUATOR_ON_OFF, &value);
	value += 10;
	value %= 256;
	DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Light sensor): 'Sensed' dummy value: %d\n", value);
	wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_LIGHT_SENSOR_CURRENT_VALUE, value);
}
