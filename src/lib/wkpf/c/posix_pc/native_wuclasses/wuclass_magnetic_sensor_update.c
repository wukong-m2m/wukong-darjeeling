#include <stdio.h>
#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include "../posix_pc_utils.h"

void wuclass_magnetic_sensor_setup(wuobject_t *wuobject) {
}

void wuclass_magnetic_sensor_update(wuobject_t *wuobject) {
	bool value = (bool)posix_property_get(wuobject, "magnetic_sensor");
	printf("WKPFUPDATE(MagneticSensor): Sensed binary value: %d\n", value);  
	wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_MAGNETIC_SENSOR_OUTPUT, value);  
}
