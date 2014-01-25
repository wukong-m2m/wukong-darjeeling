#include <stdio.h>
#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include "../posix_pc_utils.h"

void wuclass_pir_sensor_setup(wuobject_t *wuobject) {
}

void wuclass_pir_sensor_update(wuobject_t *wuobject) {
	bool value = (bool)posix_property_get(wuobject, "pir_sensor");
	printf("WKPFUPDATE(PirSensor): Sensed binary value: %d\n", value);  
	wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_PIR_SENSOR_CURRENT_VALUE, value);  
}
