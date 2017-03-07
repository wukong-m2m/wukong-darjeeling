#include "debug.h"
#include "native_wuclasses.h"
#include "../posix_pc_utils.h"

void wuclass_fan_setup(wuobject_t *wuobject) {
	// Just put a dummy value to make sure the file is created even if the object's not used in the FBP
	posix_property_put(wuobject, "fan", 0);
}

void wuclass_fan_update(wuobject_t *wuobject) {
	bool onOff;
	wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_FAN_ON_OFF, &onOff);
	posix_property_put(wuobject, "fan", onOff);
	DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Fan): Setting fan to: %x\n", onOff);
}
