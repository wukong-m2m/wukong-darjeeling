#include <stdio.h>
#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include "../posix_pc_utils.h"

void wuclass_magnetic_sensor_setup(wuobject_t *wuobject) {
	// Just get a value to make sure the file is created even if the object's not used in the FBP
	posix_property_get(wuobject, "magnetic_sensor");
}

void wuclass_magnetic_sensor_update(wuobject_t *wuobject) {
	bool value = (bool)posix_property_get(wuobject, "magnetic_sensor");
	DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(MagneticSensor): Sensed binary value: %d\n", value);  
	wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_MAGNETIC_SENSOR_OUTPUT, value);  
}
