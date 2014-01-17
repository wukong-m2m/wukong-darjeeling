#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include "../posix_pc_utils.h"

void wuclass_numeric_controller_setup(wuobject_t *wuobject) {
	// Just get a value to make sure the file is created even if the object's not used in the FBP
	posix_property_get(wuobject, "numeric_controller");
}

void wuclass_numeric_controller_update(wuobject_t *wuobject) {
	int value = posix_property_get(wuobject, "numeric_controller");
	printf("WKPFUPDATE(NumericController): Sensed value: %d\n", value);
	wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_NUMERIC_CONTROLLER_OUTPUT, (int16_t)value);
}
