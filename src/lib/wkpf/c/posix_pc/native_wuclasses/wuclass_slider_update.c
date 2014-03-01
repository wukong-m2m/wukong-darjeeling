#include "debug.h"
#include "../../common/native_wuclasses/native_wuclasses.h"
#include "../posix_pc_utils.h"

void wuclass_slider_setup(wuobject_t *wuobject) {
	// Just get a value to make sure the file is created even if the object's not used in the FBP
	posix_property_get(wuobject, "slider");
}

void wuclass_slider_update(wuobject_t *wuobject) {
	int value = posix_property_get(wuobject, "slider");
	if (value > 255)
		value = 255;

	// After this:
	// 0 <= value <= 255
	// We want to recalibrate this to the range between the low and high values for the slider
	int16_t low;
	int16_t high;
	wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_SLIDER_LOW_VALUE, &low);
	wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_SLIDER_HIGH_VALUE, &high);
	int16_t range = high-low;
	int16_t output = (int16_t)((int32_t)value*range/255+low);
	wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_SLIDER_OUTPUT, output);

	DEBUG_LOG(DBG_WKPFUPDATE, "WKPFUPDATE(Slider): Sensed %d, low %d, high %d, output %d\n", value, low, high, output);
}
