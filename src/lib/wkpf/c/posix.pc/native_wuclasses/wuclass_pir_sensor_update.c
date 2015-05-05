#include "debug.h"
#include "native_wuclasses.h"
#include "../posix_pc_utils.h"

void wuclass_pir_sensor_setup(wuobject_t *wuobject) {
	// Just get a value to make sure the file is created even if the object's not used in the FBP
	posix_property_get(wuobject, "pir_sensor");
}

void wuclass_pir_sensor_update(wuobject_t *wuobject) {
	bool value = (bool)posix_property_get(wuobject, "pir_sensor");
	DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(PirSensor): Sensed binary value: %d\n", value);  
	wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_PIR_SENSOR_CURRENT_VALUE, value);  
}
